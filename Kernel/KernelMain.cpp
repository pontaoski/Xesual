#include "Limine/limine.h"
#include "LocalAPIC.h"
#include "MiscFunctions.h"
#include "PhysicalMemoryManagement.h"
#include "VirtualMemoryManagement.h"
#include "Paging.h"
#include "SMP.h"
#include "ProcessManagement.h"
#include "Traps.h"
#include "printf.h"

extern char KernelStart[];
extern char KernelEnd[];

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

static volatile struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
};

static void halt() {
    for (;;) {
        __asm__ ("hlt");
    }
}

void dumpPageTable(PageTableRef root, int level = 1)
{
    constexpr auto pe2v = [](PageTableEntry p) {
        using namespace PhysicalMemoryManagement;
        return (PageTableRef)((p.hl.physicalAddress << PageShift) + HigherHalfDirectMapOffset);
    };

    for (int i = 0; i < level-1; i++) _putchar(' ');
    logfn("level %d\n", level);
    for (int i = 0; i < 512; i++) {
        if (!root[i].hl.isPresent) {
            continue;
        }
        for (int i = 0; i < level; i++) _putchar(' ');
        logfn("%d to %llx\n", i, root[i].hl.physicalAddress << PhysicalMemoryManagement::PageShift);
        if (level < 4) {
            dumpPageTable(pe2v(root[i]), level + 1);
        }
    }
}

uint64_t initByAddingUsablePagesToMemoryManagement()
{
    /// We use the technique of "higher half direct mapping" here, which is
    /// essentially just having all of physical memory mapped to the higher half
    /// of the virtual address space. That allows us to have access to all of physical memory
    /// no matter what we're doing with the lower half of memory that we give to userspace.
    ///
    /// Limine is kind enough to set that up for us, so we need to make sure to use the
    /// offset (virt - phys = offset) it set it up as.
    PhysicalMemoryManagement::HigherHalfDirectMapOffset = hhdm_request.response->offset;

    /// If this were a userspace program, that would be a constructor. But we're going from
    /// 0 here, so we need to explicitly initialize it.
    PhysicalMemoryManagement::init();

    uint64_t maxPhysicalMemory = 0;

    /// This loop serves two purposes:
    /// - tell our physical memory management subsystem about the pages that it's free to use
    /// - to figure out the last address of physical memory, so that we can set up our own
    ///   higher half direct mapping when we take off Limine's training wheels
    for (auto i = 0UL; i < memmap_request.response->entry_count; i++) {
        auto entry = memmap_request.response->entries[i];
        if ((entry->base + entry->length) > maxPhysicalMemory) {
            maxPhysicalMemory = entry->base + entry->length;
        }

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            using namespace PhysicalMemoryManagement;
            freeRange((void*)(entry->base + HigherHalfDirectMapOffset), (void*)(entry->base + entry->length + HigherHalfDirectMapOffset));
        }
    }

    /// Return the maximum physical memory for later usagee in setting up virtual memory
    return maxPhysicalMemory;
}

void checkThatLimineGaveUsEverythingWeNeed()
{
    if (memmap_request.response == nullptr || memmap_request.response->entry_count < 1) {
        halt();
    }
    if (hhdm_request.response == nullptr) {
        halt();
    }
    if (kernel_address_request.response == nullptr) {
        halt();
    }
    if (smp_request.response == nullptr) {
        halt();
    }
}

void initializeVirtualMemory(uint64_t maxPhysicalMemory)
{
    using namespace PhysicalMemoryManagement;
    using namespace VirtualMemoryManagement;

    /// Let's build up a new page table for the kernel
    KernelPageTable = allocateRootPageTable();

    /// Directly map all of physical memory to the higher half of the virtual address space for convenient access
    for (auto i = 0UL; i < maxPhysicalMemory; i += PageSize) {
        map(KernelPageTable, i + HigherHalfDirectMapOffset, i);
    }

    /// Let's also put the kernel code (this code, that we're currently running) into the new page table.
    /// Things might explode if the kernel code spotaneously disappears from virtual memory space.
    const auto resp = kernel_address_request.response;
    for (auto i = (uint64_t)KernelStart; i < (uint64_t)KernelEnd; i += PageSize) {
        map(KernelPageTable, i, resp->physical_base + (i - resp->virtual_base));
    }

    /// We're a higher-half kernel, so the top half of the page table belongs exclusively to the kernel.
    /// The kernel pages need to be present in every page table (including page tables for userspace code)
    /// since stuff like interrupts and syscalls transfer control directly to kernel code regardless of which page table is active.
    /// Thus, we use the kernel page table as a starting point for all userspace page tables.
    /// This is fine (sans speculative execution exploits) because of the ring-level protection offered by paging.
    /// Since we'd like to avoid having to update every page table in existence if we need a new top-level
    /// entry (the ones that are copied by value when shallow-cloning), we go ahead and preallocate the higher-half
    /// page table entries here.
    for (int i = 256; i < 512; i++) {
        if (!KernelPageTable[i].hl.isPresent) {
            allocatePageTableForTableEntry(&KernelPageTable[i]);
        }
    }

    /// This hunk of assembly is us taking off the training wheels from Limine, our bootloader:
    /// we write the physical address of the page table to the register cr3 to tell the CPU that this
    /// is the page table that we would like to use now
    asm volatile("movq %0, %%cr3" :: "r" (
        toPhysical(HHVAddress{(uint64_t)KernelPageTable})
    ));
}

extern "C" void OtherProcessorMain(limine_smp_info* info);

void initializeProcessors() {
    using namespace ProcessManagement;

    auto resp = smp_request.response;
    CPUCount = resp->cpu_count;
    CPUs = (CPUState*)PhysicalMemoryManagement::allocatePage().asPtr();
    memset((void*)CPUs, 0, PhysicalMemoryManagement::PageSize);

    // enable using the lock to ensure that we don't explode when
    // more than one processor wants to allocate a page
    PhysicalMemoryManagement::thereIsNowMoreThanOneProcessor();

    for (auto i = 0UL; i < resp->cpu_count; i++) {
        CPUs[i].apicID = resp->cpus[i]->lapic_id;
        CPUs[i].cliCount = 1;
        CPUs[i].currentProcess = nullptr;

        if (resp->cpus[i]->lapic_id == resp->bsp_lapic_id)
            continue;

        __atomic_store_n(&resp->cpus[i]->goto_address, OtherProcessorMain, __ATOMIC_RELAXED);
    }
}

void disableAllLegacyInterrupts()
{
    // mask all interrupts from the 8259A controllers
    outb(0x20+1, 0xFF);
    outb(0xA0+1, 0xFF);
}

void SharedMain() {
    logfn("stack! %d\n", LocalAPIC::lapicID());
    ProcessManagement::thisCPU()->stack = PhysicalMemoryManagement::allocatePages(ProcessManagement::StackSize).asPtr();
    logfn("init! %d\n", LocalAPIC::lapicID());
    ProcessManagement::thisCPU()->table.init((uintptr_t)ProcessManagement::thisCPU()->stack);
    logfn("install! %d\n", LocalAPIC::lapicID());
    ProcessManagement::thisCPU()->table.install();
    logfn("gsbase! %d\n", LocalAPIC::lapicID());
    ProcessManagement::thisCPU()->setKernelGSBase();
    logfn("syscall rip! %d\n", LocalAPIC::lapicID());
    ProcessManagement::thisCPU()->setSyscallRIP();
    logfn("star register! %d\n", LocalAPIC::lapicID());
    ProcessManagement::thisCPU()->setStarRegister();

    Traps::loadIDTTable();
    ProcessManagement::popCLI();

    if (LocalAPIC::lapicID() == 0) {
        logfn("starting first process...\n");
        ProcessManagement::runFirstProcess();
    }
    // halt();

    logfn("Processor %d is up and running!\n", LocalAPIC::lapicID());
    halt();
}

extern "C" void OtherProcessorMain(limine_smp_info*) {
    using namespace PhysicalMemoryManagement;
    using namespace VirtualMemoryManagement;

    // we should get the other processors all using the same table
    asm volatile("movq %0, %%cr3" :: "r" (
        toPhysical(HHVAddress{(uint64_t)KernelPageTable})
    ));

    SMP::setAPICBase();
    LocalAPIC::initLAPIC();

    SharedMain();
}

extern "C" void initLogLock();

extern "C" void KernelMain() {
    using namespace PhysicalMemoryManagement;
    using namespace VirtualMemoryManagement;

    checkThatLimineGaveUsEverythingWeNeed();
    const auto maxPhysicalAddress = initByAddingUsablePagesToMemoryManagement();
    initializeVirtualMemory(maxPhysicalAddress);

    initLogLock();
    SMP::setAPICBase();
    LocalAPIC::setLAPICAddress();
    Traps::setupIDTTable();
    initializeProcessors();

    disableAllLegacyInterrupts();
    LocalAPIC::initLAPIC();

    SharedMain();
}
