#include "limine.h"
#include "utils.h"
#include "types.h"
#include "string.h"
#include "interrupt.h"
#include "display.h"
#include "gdt.h"

__attribute__((used, section(".limine_requests")))
static volatile uint64_t limine_base_revision[] = LIMINE_BASE_REVISION(4);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST_ID,
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[] = LIMINE_REQUESTS_END_MARKER;

void memmap_entries() {
    if (memmap_request.response == NULL
        || memmap_request.response->entry_count < 1) {
            hcf();
    }

    u64 memmap_entry_count = memmap_request.response->entry_count;
    struct limine_memmap_entry** memmap_entries = memmap_request.response->entries;

    u8 buffer[128];
    for (u64 i = 0; i < memmap_entry_count; i++) {
        print(str8_lit("memmap entry: base="));
        print(u64_to_str8_hex(memmap_entries[i]->base, buffer, 128));
        print(str8_lit(" length="));
        print(u64_to_str8_hex(memmap_entries[i]->length, buffer, 128));
        print(str8_lit(" type="));
        print(u64_to_str8_hex(memmap_entries[i]->type, buffer, 128));
        print(str8_lit("\n"));
    }
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED(limine_base_revision) == false) {
        hcf();
    }

    init_display();

    load_gdt();

    init_interrupts();

    hcf();
}
