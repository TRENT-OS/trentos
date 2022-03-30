//------------------------------------------------------------------------------
//
// seL4 native hello world application
//
// Copyright (C) 2019, HENSOLDT Cyber GmbH
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <assert.h>
#include <sel4/sel4.h>
#include <sel4runtime.h>
#include <sel4debug/debug.h>

static void print_slot_reg_info(char const *descr, seL4_SlotRegion *reg)
{
    if (reg->end == reg->start) {
        printf("%snone\n", descr);
    } else {
        printf("%s[%"SEL4_PRIu_word" --> %"SEL4_PRIu_word")\n",
               descr, reg->start, reg->end);
    }
}

void show_custom_boot_info(seL4_BootInfo *info)
{
    printf("Node:                      %"SEL4_PRIu_word" of %"SEL4_PRIu_word"\n",
           info->nodeID, info->numNodes);
    printf("Domain:                    %"SEL4_PRIu_word"\n", info->initThreadDomain);
    printf("IPC buffer:                %p\n", info->ipcBuffer);
    printf("IO-MMU PT levels:          %"SEL4_PRIu_word"\n",
           info->numIOPTLevels);
    printf("Root cnode size:           2^%"SEL4_PRIu_word" (%lu)\n",
           info->initThreadCNodeSizeBits,
           LIBSEL4_BIT(info->initThreadCNodeSizeBits));
    print_slot_reg_info("Shared pages:              ", &info->sharedFrames);
    print_slot_reg_info("User image MMU tables:     ", &info->userImagePaging);
    print_slot_reg_info("Extra boot info pages:     ", &info->extraBIPages);
    print_slot_reg_info("User image pages:          ", &info->userImageFrames);
    print_slot_reg_info("Untyped memory:            ", &info->untyped);
    print_slot_reg_info("Empty slots:               ", &info->empty);
    seL4_Word numUntypeds = info->untyped.end - info->untyped.start;
    printf("Used bootinfo untyped descriptors: %"SEL4_PRIu_word" of %d\n",
           numUntypeds, CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS);
    printf("Extra boot info blobs (len: %"SEL4_PRIu_word")\n", info->extraLen);

    /* Sanity check that the boot info is consistent. */
    assert(info->empty.end == LIBSEL4_BIT(info->initThreadCNodeSizeBits));
    assert(numUntypeds < CONFIG_MAX_NUM_BOOTINFO_UNTYPED_CAPS);
    assert(info->extraLen <= (LIBSEL4_BIT(seL4_PageBits) * (info->extraBIPages.end - info->extraBIPages.start)));

    uintptr_t base = (uintptr_t)info + seL4_BootInfoFrameSize;
    uintptr_t offs = 0;
    while (offs < info->extraLen) {
        seL4_BootInfoHeader *h = (seL4_BootInfoHeader *)(base + offs);
        // SEL4_BOOTINFO_HEADER_PADDING            = 0,
        // SEL4_BOOTINFO_HEADER_X86_VBE            = 1,
        // SEL4_BOOTINFO_HEADER_X86_MBMMAP         = 2,
        // SEL4_BOOTINFO_HEADER_X86_ACPI_RSDP      = 3,
        // SEL4_BOOTINFO_HEADER_X86_FRAMEBUFFER    = 4,
        // SEL4_BOOTINFO_HEADER_X86_TSC_FREQ       = 5, /* frequency is in MHz */
        // SEL4_BOOTINFO_HEADER_FDT                = 6, /* device tree */
        // /* add more IDs here */
        // SEL4_BOOTINFO_HEADER_NUM
        printf("    type: %"SEL4_PRIu_word", offset: %"PRIu_PTR", len: %"SEL4_PRIu_word"\n",
               h->id, offs, h->len);
        offs += h->len;
        assert(offs <= info->extraLen);
    }


    printf("Physical memory map with available untypeds:\n");

    /* enough chars to fill space used by an u64 in hex */
    static const char *DASHES = "----------------";
    static const char *SPACES = "                ";

    printf("%.*s-------------+------+----------+-------\n",
           (CONFIG_WORD_SIZE / 4) - 8, DASHES);
    printf("%.*s  Phys Addr  | Size | Type     | Slot\n",
           (CONFIG_WORD_SIZE / 4) - 8, SPACES);
    printf("%.*s-------------+------+----------+-------\n",
           (CONFIG_WORD_SIZE / 4) - 8, DASHES);
    seL4_Word pos = 0;
    seL4_Word cnt = 0;
    for (;;) {
        /* Find the next descriptor according to our current position. */
        seL4_UntypedDesc *d = NULL;
        int idx = -1;
        for (int i = 0; i < numUntypeds; i++) {
            seL4_UntypedDesc *d_tmp = &info->untypedList[i];
            if (d_tmp->paddr < pos) {
                continue;
            }
            if (d && (d_tmp->paddr >= d->paddr)) {
                /* Two  descriptors for the same region are not allowed. */
                assert(d_tmp->paddr != d->paddr);
                continue;
            }
            d = d_tmp; /* Found a better match. */
            idx = i;
        }
        if (!d || (pos < d->paddr)) {
            /* No adjacent descriptor means there is reserved space. */
            seL4_Word size = (d ? d->paddr: -1) - pos;
            if (size > 1)
            {
                printf("  %#*"SEL4_PRIx_word" | -    | reserved | -\n",
                       2 + (CONFIG_WORD_SIZE / 4), pos);
            }
            if (!d) {
                break; /* No descriptors found at all means we are done. */
            }
        }
        cnt++; /* found a descriptor */
        printf("  %#*"SEL4_PRIx_word" | 2^%-2u | %s | %"SEL4_PRId_word"\n",
               2 + (CONFIG_WORD_SIZE / 4),
               d->paddr,
               d->sizeBits,
               d->isDevice ? "device  " : "free    ",
               info->untyped.start + idx);

        /* Check if end of phys address space has been reached. */
        seL4_Word size = LIBSEL4_BIT(d->sizeBits);
        seL4_Word remaining = (seL4_Word)(-1) - d->paddr;
        if (size > remaining) {
            break;
        }
        pos = d->paddr + size;
    }
    assert(numUntypeds == cnt); /* all untypeds could be matched */
}

//------------------------------------------------------------------------------
static void test_benchmark(void)
{
#ifdef CONFIG_ENABLE_BENCHMARKS
    // seL4_Error ret;
    // ret = seL4_BenchmarkResetLog();
    // ret = seL4_BenchmarkFinalizeLog();
    // ret = seL4_BenchmarkSetLogBuffer(seL4_Word frame_cptr)

    printf("seL4_BenchmarkNullSyscall...\n");
    seL4_BenchmarkNullSyscall();

    printf("seL4_BenchmarkFlushCaches...\n");
    seL4_BenchmarkFlushCaches();

    // seL4_BenchmarkFlushL1Caches(seL4_Word cache_type)
    // seL4_BenchmarkGetThreadUtilisation(seL4_Word tcb_cptr)
    // seL4_BenchmarkResetThreadUtilisation(seL4_Word tcb_cptr)

#ifdef CONFIG_BENCHMARK_TRACK_UTILISATION
    printf("seL4_BenchmarkResetAllThreadsUtilisation...\n");
    seL4_BenchmarkResetAllThreadsUtilisation();
    printf("seL4_Yield...\n");
    seL4_Yield();
    printf("Test\n");
    printf("seL4_Yield...\n");
    seL4_Yield();
#ifdef CONFIG_DEBUG_BUILD
    printf("seL4_BenchmarkDumpAllThreadsUtilisation...\n");
    seL4_BenchmarkDumpAllThreadsUtilisation();
#endif /* CONFIG_DEBUG_BUILD */
#endif /* CONFIG_BENCHMARK_TRACK_UTILISATION  */
#endif /* CONFIG_ENABLE_BENCHMARKS */
}


//------------------------------------------------------------------------------
int main(void)
{
    printf("hello word main\n");

    seL4_BootInfo *info = sel4runtime_bootinfo();
    // debug_print_bootinfo(info);
    show_custom_boot_info(info);

    test_benchmark();

    printf("Hello, world!\n");
    seL4_DebugHalt();

    return 0;
}
