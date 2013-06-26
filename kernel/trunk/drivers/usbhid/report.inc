; Parser of HID structures: parse HID report descriptor,
; parse/generate input/output/feature reports.

; =============================================================================
; ================================= Constants =================================
; =============================================================================
; Usage codes from HID specification
; Generic Desktop usage page
USAGE_GD_POINTER     = 10001h
USAGE_GD_MOUSE       = 10002h
USAGE_GD_JOYSTICK    = 10004h
USAGE_GD_GAMEPAD     = 10005h
USAGE_GD_KEYBOARD    = 10006h
USAGE_GD_KEYPAD      = 10007h

USAGE_GD_X           = 10030h
USAGE_GD_Y           = 10031h
USAGE_GD_Z           = 10032h
USAGE_GD_RX          = 10033h
USAGE_GD_RY          = 10034h
USAGE_GD_RZ          = 10035h
USAGE_GD_SLIDER      = 10036h
USAGE_GD_DIAL        = 10037h
USAGE_GD_WHEEL       = 10038h

; Keyboard/Keypad usage page
USAGE_KBD_NOEVENT    = 70000h
USAGE_KBD_ROLLOVER   = 70001h
USAGE_KBD_POSTFAIL   = 70002h
USAGE_KBD_FIRST_KEY  = 70004h ; this is 'A', actually
USAGE_KBD_LCTRL      = 700E0h
USAGE_KBD_LSHIFT     = 700E1h
USAGE_KBD_LALT       = 700E2h
USAGE_KBD_LWIN       = 700E3h
USAGE_KBD_RCTRL      = 700E4h
USAGE_KBD_RSHIFT     = 700E5h
USAGE_KBD_RALT       = 700E6h
USAGE_KBD_RWIN       = 700E7h

; LED usage page
USAGE_LED_NUMLOCK    = 80001h
USAGE_LED_CAPSLOCK   = 80002h
USAGE_LED_SCROLLLOCK = 80003h

; Button usage page
; First button is USAGE_BUTTON_PAGE+1, second - USAGE_BUTTON_PAGE+2 etc.
USAGE_BUTTON_PAGE    = 90000h

; Flags for input/output/feature fields
HID_FIELD_CONSTANT   = 1 ; if not, then Data field
HID_FIELD_VARIABLE   = 2 ; if not, then Array field
HID_FIELD_RELATIVE   = 4 ; if not, then Absolute field
HID_FIELD_WRAP       = 8
HID_FIELD_NONLINEAR  = 10h
HID_FIELD_NOPREFERRED= 20h ; no preferred state
HID_FIELD_HASNULL    = 40h ; has null state
HID_FIELD_VOLATILE   = 80h ; for output/feature fields
HID_FIELD_BUFBYTES   = 100h; buffered bytes

; Report descriptor can easily describe gigabytes of (meaningless) data.
; Keep report size reasonable to avoid excessive memory allocations and
; calculation overflows; 1 Kb is more than enough (typical size is 3-10 bytes).
MAX_REPORT_BYTES = 1024

; =============================================================================
; ================================ Structures =================================
; =============================================================================
; Every meaningful report field group has one or more associated usages.
; Usages can be individual or joined into continuous ranges.
; This structure describes one range or one individual usage in a large array;
; individual usage is equivalent to a range of length 1.
struct usage_range
offset          dd      ?
; Sum of range sizes over all previous array items.
; Size of range a equals
; [a + sizeof.usage_range + usage_range.offset] - [a + usage_range.offset].
; The total sum over all array items immediately follows the array,
; this field must be the first so that the formula above works for the last item.
first_usage     dd      ?
; Usage code for first item in the range.
ends

; This structure describes one group of report fields with identical properties.
struct report_field_group
next            dd      ?
; All field groups in one report are organized in a single-linked list.
; This is the next group in the report or 0 for the last group.
size            dd      ?
; Size in bits of one field. Cannot be zero or greater than 32.
count           dd      ?       ; field count, cannot be zero
offset          dd      ?       ; offset from report start, in bits
; Following fields are decoded from report descriptor, see HID spec for details.
flags           dd      ?
logical_minimum dd      ?
logical_maximum dd      ?
physical_minimum dd     ?
physical_maximum dd     ?
unit_exponent   dd      ?
unit            dd      ?
; Following fields are used to speedup extract_field_value.
mask            dd      ?
; Bitmask for all data bits except sign bit:
; (1 shl .size) - 1 for unsigned fields, (1 shl (.size-1)) - 1 for signed fields
sign_mask       dd      ?
; Zero for unsigned fields. Bitmask with sign bit set for signed fields.
common_sizeof   rd      0
; Variable and Array field groups differ significantly.
; Variable field groups are simple. There are .count fields, each field has
; predefined Usage, the content of a field is its value. Each field is
; always present in the report. For Variable field groups, we just keep
; additional .count dwords with usages for individual fields.
; Array field groups are complicated. There are .count uniform fields.
; The content of a field determines Usage; Usages which are currently presented
; in the report have value = 1, other Usages have value = 0. The number of
; possible Usages is limited only by field .size; 32-bit field could encode any
; Usage, so it is unreasonable to keep all Usages in the plain array, as with
; Variable fields. However, many unrelated Usages in one group are meaningless,
; so usually possible values are grouped in sequential ranges; number of ranges
; is limited by report descriptor size (max 0xFFFF bytes should contain all
; information, including usage ranges and field descriptions).
; Also, for Array variables we pass changes in state to drivers, not the state
; itself, because sending information about all possible Usages is inpractical;
; so we should remember the previous state in addition to the current state.
; Thus, for Array variables keep the following information, in this order:
; * some members listed below; note that they do NOT exist for Variable groups;
; * array of usage ranges in form of usage_range structures, including
;   an additional dword after array described in usage_range structure;
; * allocated memory for current values of the report;
; * values of the previous report.
num_values_prev dd      ?       ; number of values in the previous report
num_usage_ranges dd     ?       ; number of usage_range, always nonzero
usages          rd      0
ends

; This structure describes one report.
; All reports of one type are organized into a single-linked list.
struct report
next            dd      ?       ; pointer to next report of the same type, if any
size            dd      ?       ; total size in bits
first_field     dd      ?       ; pointer to first report_field_group for this report
last_field      dd      ?
; pointer to last report_field_group for this report, if any;
; address of .first_field, if .first_field is 0
id              dd      ?
; Report ID, if assigned. Zero otherwise.
top_level_collection dd ?       ; top-level collection for this report
ends

; This structure describes a set of reports of the same type;
; there are 3 sets (possibly empty), input, output and feature.
struct report_set
data            dd      ?
; If .numbered is zero, this is zero for the empty set and
; a pointer to the (only) report structure otherwise.
; If .numbered is nonzero, this is a pointer to 256-dword array of pointers
; to reports organized by report ID.
first_report    dd      ?
; Pointer to the first report or 0 for the empty set.
numbered        db      ?
; If zero, report IDs are not used, there can be at most one report in the set.
; If nonzero, first byte of the report is report ID.
                rb      3       ; padding
ends

; This structure describes a range of reports of one type that belong to
; some collection.
struct collection_report_set
first_report    dd      ?
first_field     dd      ?
last_report     dd      ?
last_field      dd      ?
ends

; This structure defines driver callbacks which are used while
; device is active; i.e. all callbacks except add_device.
struct hid_driver_active_callbacks
disconnect      dd      ?
; Called when an existing HID device is disconnected.
;
; Four following functions are called when a new input packet arrives
; in the following order: .begin_packet, then .input_field several times
; for each input field, interleaved with .array_overflow? for array groups,
; then .end_packet.
begin_packet    dd      ?
; edi -> driver data
array_overflow? dd      ?
; edi -> driver data
; out: CF cleared <=> ignore this array
input_field     dd      ?
; edi -> driver data, ecx = usage, edx = value
end_packet      dd      ?
; edi -> driver data
ends

; This structure describes one collection.
struct collection
next            dd      ?       ; pointer to the next collection in the same level
                                ; must be the first field
parent          dd      ?       ; pointer to nesting collection
first_child     dd      ?       ; pointer to the first nested collection
last_child      dd      ?       ; pointer to the last nested collection
                                ; or to .first_child, if .first_child is zero
type            dd      ?       ; Application, Physical etc
usage           dd      ?       ; associated Usage code
; Next fields are filled only for top-level collections.
callbacks       hid_driver_active_callbacks
driver_data     dd      ?       ; value to be passed as is to driver callbacks
input           collection_report_set
output          collection_report_set
feature         collection_report_set
ends

; This structure keeps all data used by the HID layer for one device.
struct hid_data
input           report_set
output          report_set
feature         report_set
first_collection        dd      ?
ends

; This structure defines callbacks required from the driver.
struct hid_driver_callbacks
add_device      dd      ?
; Called when a new HID device is connected.
active          hid_driver_active_callbacks
ends

; Two following structures describe temporary data;
; the corresponding objects cease to exist when HID parser completes
; state of Global items
struct global_items
next                    dd      ?
usage_page              dd      ?
logical_minimum         dd      ?
logical_maximum         dd      ?
physical_minimum        dd      ?
physical_maximum        dd      ?
unit_exponent           dd      ?
unit                    dd      ?
report_size             dd      ?
report_id               dd      ?
report_count            dd      ?
ends

; one range of Usages
struct usage_list_item
next                    dd      ?
first_usage             dd      ?
num_usages              dd      ?
ends

; =============================================================================
; =================================== Code ====================================
; =============================================================================

macro workers_globals
{
        workers_globals
; Jump tables for switch'ing in the code.
align 4
; jump table for two lower bits which encode size of item data
parse_descr_label.fetch_jumps:
        dd      parse_descr_label.fetch_none    ; x0, x4, x8, xC
        dd      parse_descr_label.fetch_byte    ; x1, x5, x9, xD
        dd      parse_descr_label.fetch_word    ; x2, x6, xA, xE
        dd      parse_descr_label.fetch_dword   ; x3, x7, xB, xF
; jump table for two next bits which encode item type
parse_descr_label.type_jumps:
        dd      parse_descr_label.parse_main
        dd      parse_descr_label.parse_global
        dd      parse_descr_label.parse_local
        dd      parse_descr_label.parse_reserved
; jump table for 4 upper bits in the case of Main item
parse_descr_label.main_jumps:
        dd      parse_descr_label.input ; 80...83
        dd      parse_descr_label.output        ; 90...93
        dd      parse_descr_label.collection    ; A0...A3
        dd      parse_descr_label.feature       ; B0...B3
        dd      parse_descr_label.end_collection ; C0...C3
parse_descr_label.num_main_items = ($ - parse_descr_label.main_jumps) / 4
; jump table for 4 upper bits in the case of Global item
parse_descr_label.global_jumps:
        dd      parse_descr_label.usage_page    ; 04...07
        dd      parse_descr_label.logical_minimum ; 14...17
        dd      parse_descr_label.logical_maximum ; 24...27
        dd      parse_descr_label.physical_minimum ; 34...37
        dd      parse_descr_label.physical_maximum ; 44...47
        dd      parse_descr_label.unit_exponent ; 54...57
        dd      parse_descr_label.unit          ; 64...67
        dd      parse_descr_label.report_size   ; 74...77
        dd      parse_descr_label.report_id     ; 84...87
        dd      parse_descr_label.report_count  ; 94...97
        dd      parse_descr_label.push          ; A4...A7
        dd      parse_descr_label.pop           ; B4...B7
parse_descr_label.num_global_items = ($ - parse_descr_label.global_jumps) / 4
; jump table for 4 upper bits in the case of Local item
parse_descr_label.local_jumps:
        dd      parse_descr_label.usage ; 08...0B
        dd      parse_descr_label.usage_minimum ; 18...1B
        dd      parse_descr_label.usage_maximum ; 28...2B
        dd      parse_descr_label.item_parsed   ; 38...3B = designator item; ignore
        dd      parse_descr_label.item_parsed   ; 48...4B = designator minimum; ignore
        dd      parse_descr_label.item_parsed   ; 58...5B = designator maximum; ignore
        dd      parse_descr_label.item_parsed   ; 68...6B not assigned
        dd      parse_descr_label.item_parsed   ; 78...7B = string index; ignore
        dd      parse_descr_label.item_parsed   ; 88...8B = string minimum; ignore
        dd      parse_descr_label.item_parsed   ; 98...9B = string maximum; ignore
        dd      parse_descr_label.delimiter     ; A8...AB
parse_descr_label.num_local_items = ($ - parse_descr_label.local_jumps) / 4
}

; Local variables for parse_descr.
macro parse_descr_locals
{
cur_item_size   dd      ?       ; encoded size of data for current item
report_ok       db      ?       ; 0 on error, 1 if everything is ok
field_type      db      ?       ; 0/1/2 for input/output/feature fields
                rb      2       ; alignment
field_data      dd      ?       ; data for current item when it describes a field group
last_reports    rd      3       ; pointers to last input/output/feature records
usage_minimum   dd      ?       ; current value of Usage Minimum
usage_list      dd      ?       ; list head of usage_list_item
usage_tail      dd      ?       ; list tail of usage_list_item
num_usage_ranges dd     ?       ; number of usage ranges, size of usage_list
delimiter_depth dd      ?       ; normally 0; 1 inside of Delimiter();
                                ; nested Delimiter()s are not allowed
usage_variant   dd      ?       ; 0 outside of Delimiter()s and for first Usage inside Delimiter(),
                                ; incremented with each new Usage inside Delimiter()
cur_collection  dd      ?       ; current collection
last_collection dd      ?       ; last top-level collection
}

; Parse report descriptor. The caller should provide local variables
; [buffer] = pointer to report descriptor, [length] = length of report descriptor,
; [calldata] = pointer to hid_data (possibly wrapped in a large structure).
macro parse_descr
{
parse_descr_label:
; 1. Initialize.
; 1a. Set some variables to initial values.
        xor     edi, edi
        mov     dword [report_ok], edi
        mov     [usage_list], edi
        mov     [cur_collection], edi
        mov     eax, [calldata]
        add     eax, hid_data.input.first_report
        mov     [last_reports+0*4], eax
        add     eax, hid_data.output.first_report - hid_data.input.first_report
        mov     [last_reports+1*4], eax
        add     eax, hid_data.feature.first_report - hid_data.output.first_report
        mov     [last_reports+2*4], eax
        add     eax, hid_data.first_collection - hid_data.feature.first_report
        mov     [last_collection], eax
; 1b. Allocate state of global items.
        movi    eax, sizeof.global_items
        call    Kmalloc
        test    eax, eax
        jz      .memory_error
; 1c. Zero-initialize it and move pointer to edi.
        push    eax
        xchg    eax, edi
        movi    ecx, sizeof.global_items / 4
        rep stosd
        pop     edi
; 1d. Load pointer to data into esi and make [length] point to end of data.
        mov     esi, [buffer]
        add     [length], esi
; 2. Clear all local items.
; This is needed in the beginning and after processing any Main item.
.zero_local_items:
        mov     eax, [usage_list]
@@:
        test    eax, eax
        jz      @f
        push    [eax+usage_list_item.next]
        call    Kfree
        pop     eax
        jmp     @b
@@:
        lea     ecx, [usage_list]
        mov     [usage_tail], ecx
        mov     [ecx], eax
        mov     [delimiter_depth], eax
        mov     [usage_variant], eax
        mov     [usage_minimum], eax
        mov     [num_usage_ranges], eax
; 3. Parse items until end of data found.
        cmp     esi, [length]
        jae     .parse_end
.fetch_next_item:
; --------------------------------- Parse item --------------------------------
; 4. Parse one item.
; 4a. Get item data. eax = first item byte = code+type+size (4+2+2 bits),
; ebx = item data interpreted as unsigned,
; ecx = item data interpreted as signed.
        movzx   eax, byte [esi]
        mov     ecx, eax
        and     ecx, 3
        mov     [cur_item_size], ecx
        jmp     dword [.fetch_jumps+ecx*4]
.invalid_report:
        mov     esi, invalid_report_msg
        jmp     .end_str
.fetch_none:
        xor     ebx, ebx
        xor     ecx, ecx
        inc     esi
        jmp     .fetched
.fetch_byte:
        add     esi, 2
        cmp     esi, [length]
        ja      .invalid_report
        movzx   ebx, byte [esi-1]
        movsx   ecx, bl
        jmp     .fetched
.fetch_word:
        add     esi, 3
        cmp     esi, [length]
        ja      .invalid_report
        movzx   ebx, word [esi-2]
        movsx   ecx, bx
        jmp     .fetched
.fetch_dword:
        add     esi, 5
        cmp     esi, [length]
        ja      .invalid_report
        mov     ebx, dword [esi-4]
        mov     ecx, ebx
.fetched:
; 4b. Select the branch according to item type.
; For every type, select the concrete handler and go there.
        mov     edx, eax
        shr     edx, 2
        and     edx, 3
        shr     eax, 4
        jmp     dword [.type_jumps+edx*4]
; -------------------------------- Main items ---------------------------------
.parse_main:
        sub     eax, 8
        cmp     eax, .num_main_items
        jae     .item_parsed
        jmp     dword [.main_jumps+eax*4]
; There are 5 Main items.
; Input/Output/Feature items create new field groups in the corresponding report;
; Collection item opens a new collection (possibly nested),
; End Collection item closes the most nested collection.
.output:
        mov     [field_type], 1
        jmp     .new_field
.feature:
        mov     [field_type], 2
        jmp     .new_field
.input:
        mov     [field_type], 0
.new_field:
; Create a new field group.
        mov     [field_data], ebx
        movzx   ebx, [field_type]
if sizeof.report_set = 12
        lea     ebx, [ebx*3]
        shl     ebx, 2
else
err Change the code
end if
        add     ebx, [calldata]
; 5. Sanity checks: field size and fields count must be nonzero,
; field size cannot be more than 32 bits,
; if field count is more than MAX_REPORT_SIZE * 8, the report would be more than
; MAX_REPORT_SIZE bytes, so it is invalid too.
; More precise check for size occurs later; this check only guarantees that
; there will be no overflows during subsequent calculations.
        cmp     [edi+global_items.report_size], 0
        jz      .invalid_report
        cmp     [edi+global_items.report_size], 32
        ja      .invalid_report
; There are devices with Report Count(0) + Input(Constant Variable),
; zero-length padding. Thus, do not consider descriptors with Report Count(0)
; as invalid; instead, just ignore fields with Report Count(0).
        cmp     [edi+global_items.report_count], 0
        jz      .zero_local_items
        cmp     [edi+global_items.report_count], MAX_REPORT_BYTES * 8
        ja      .invalid_report
; 6. Get the pointer to the place for the corresponding report in ebx.
; 6a. If report ID is not assigned, ebx already points to report_set.data,
; so go to 7.
        cmp     [edi+global_items.report_id], 0
        jz      .report_ptr_found
; 6b. If table for reports was already allocated,
; go to 6d skipping the next substep.
        cmp     [ebx+report_set.numbered], 0
        jnz     .report_set_allocated
; 6c. This is the first report with ID;
; allocate and zero-initialize table for reports.
; Note: it is incorrect but theoretically possible that some fields were
; already allocated in report without ID; if so, abort processing with error.
        cmp     [ebx+report_set.data], 0
        jnz     .invalid_report
        mov     eax, 256*4
        call    Kmalloc
        test    eax, eax
        jz      .memory_error
        mov     [ebx+report_set.data], eax
        inc     [ebx+report_set.numbered]
        push    edi
        mov     edi, eax
        mov     ecx, 256
        xor     eax, eax
        rep stosd
        pop     edi
; 6d. Report ID is assigned, report table is allocated,
; get the pointer to the corresponding item in the report table.
.report_set_allocated:
        mov     ebx, [ebx+report_set.data]
        mov     ecx, [edi+global_items.report_id]
        lea     ebx, [ebx+ecx*4]
; 7. If the field group is the first one in the report,
; allocate and initialize report without fields.
.report_ptr_found:
; 7a. Check whether the report has been allocated.
        cmp     dword [ebx], 0
        jnz     .report_allocated
; 7b. Allocate.
        movi    eax, sizeof.report
        call    Kmalloc
        test    eax, eax
        jz      .memory_error
; 7c. Initialize.
        xor     edx, edx
        lea     ecx, [eax+report.first_field]
        mov     [ebx], eax
        mov     [eax+report.next], edx
        mov     [eax+report.size], edx
        mov     [ecx], edx
        mov     [eax+report.last_field], ecx
        mov     [eax+report.top_level_collection], edx
        mov     ecx, [edi+global_items.report_id]
        mov     [eax+report.id], ecx
; 7d. Append to the overall list of reports.
        movzx   edx, [field_type]
        lea     edx, [last_reports+edx*4]
        mov     ecx, [edx]
        mov     [edx], eax
        mov     [ecx], eax
.report_allocated:
        mov     ebx, [ebx]
; ebx points to an already existing report; add new field.
; 8. Calculate total size of the group and
; check that the new group would not overflow the report.
        mov     eax, [edi+global_items.report_size]
        mul     [edi+global_items.report_count]
        mov     ecx, [ebx+report.size]
        add     ecx, eax
        cmp     ecx, MAX_REPORT_BYTES * 8
        ja      .invalid_report
; 9. If there are no usages for this group, this is padding;
; add it's size to total report size and stop processing.
        cmp     [num_usage_ranges], 0
        jz      .padding
; 10. Allocate memory for the group: this includes field group structure
; and additional fields depending on field type.
; See comments in report_field_group structure.
        push    eax
        mov     edx, [edi+global_items.report_count]
        lea     eax, [report_field_group.common_sizeof+edx*4]
        test    byte [field_data], HID_FIELD_VARIABLE
        jnz     @f
        lea     eax, [eax+edx*4]
        mov     edx, [num_usage_ranges]
        lea     eax, [eax+edx*sizeof.usage_range+4]
@@:
        call    Kmalloc
        pop     edx
        test    eax, eax
        jz      .memory_error
; 11. Update report data.
; Field offset is the current report size;
; get the current report size and update report size.
; Also store the pointer to new field in the previous last field
; and update the last field.
        mov     ecx, [ebx+report.last_field]
        xadd    [ebx+report.size], edx
        mov     [ebx+report.last_field], eax
        mov     [ecx], eax
; 12. Initialize field data: offset was calculated in the previous step,
; copy other characteristics from global_items data,
; calculate .mask and .sign_mask.
        mov     [eax+report_field_group.offset], edx
        xor     edx, edx
        mov     [eax+report_field_group.next], edx
        mov     [eax+report_field_group.sign_mask], edx
        inc     edx
        mov     ecx, [edi+global_items.report_size]
        mov     [eax+report_field_group.size], ecx
        shl     edx, cl
        cmp     [edi+global_items.logical_minimum], 0
        jge     .unsigned
        shr     edx, 1
        mov     [eax+report_field_group.sign_mask], edx
.unsigned:
        dec     edx
        mov     [eax+report_field_group.mask], edx
        mov     ecx, [edi+global_items.report_count]
        mov     [eax+report_field_group.count], ecx
        mov     ecx, [field_data]
        mov     [eax+report_field_group.flags], ecx
irps field, logical_minimum logical_maximum physical_minimum physical_maximum unit_exponent unit
\{
        mov     ecx, [edi+global_items.\#field]
        mov     [eax+report_field_group.\#field], ecx
\}
; 13. Update the current collection; nesting collections will be updated by
; end-of-collection handler.
        movzx   edx, [field_type]
if sizeof.collection_report_set = 16
        shl     edx, 4
else
err Change the code
end if
        mov     ecx, [cur_collection]
        test    ecx, ecx
        jz      .no_collection
        lea     ecx, [ecx+collection.input+edx]
        mov     [ecx+collection_report_set.last_report], ebx
        mov     [ecx+collection_report_set.last_field], eax
        cmp     [ecx+collection_report_set.first_field], 0
        jnz     .no_collection
        mov     [ecx+collection_report_set.first_report], ebx
        mov     [ecx+collection_report_set.first_field], eax
.no_collection:
; 14. Transform usage ranges. The target format depends on field type.
        test    byte [eax+report_field_group.flags], HID_FIELD_VARIABLE
        jz      .transform_usages_for_array
; For Variable field groups, expand all ranges to array with .count Usages.
; If total number of Usages in all ranges is too large, ignore excessive.
; If total number of Usages in all ranges is too small, duplicate the last
; Usage up to .count Usages (e.g. group of several indicators can have one usage
; "Generic Indicator" assigned to all fields).
        mov     ecx, [eax+report_field_group.count]
        mov     ebx, [usage_list]
.next_usage_range_for_variable:
        mov     edx, [ebx+usage_list_item.first_usage]
        push    [ebx+usage_list_item.num_usages]
.next_usage_for_variable:
        mov     [eax+report_field_group.common_sizeof], edx
        dec     ecx
        jz      @f
        add     eax, 4
        inc     edx
        dec     dword [esp]
        jnz     .next_usage_for_variable
        dec     edx
        inc     dword [esp]
        cmp     [ebx+usage_list_item.next], 0
        jz      .next_usage_for_variable
        pop     edx
        mov     ebx, [ebx+usage_list_item.next]
        jmp     .next_usage_range_for_variable
@@:
        pop     ebx
        jmp     .zero_local_items
.transform_usages_for_array:
; For Array field groups, leave ranges unexpanded, but recode in the form
; more convenient to value lookup, see comments in report_field_group structure.
        mov     ecx, [num_usage_ranges]
        mov     [eax+report_field_group.num_usage_ranges], ecx
        and     [eax+report_field_group.num_values_prev], 0
        mov     ecx, [usage_list]
        xor     ebx, ebx
@@:
        mov     edx, [ecx+usage_list_item.first_usage]
        mov     [eax+report_field_group.usages+usage_range.offset], ebx
        add     ebx, [ecx+usage_list_item.num_usages]
        jc      .invalid_report
        mov     [eax+report_field_group.usages+usage_range.first_usage], edx
        add     eax, sizeof.usage_range
        mov     ecx, [ecx+usage_list_item.next]
        test    ecx, ecx
        jnz     @b
        mov     [eax+report_field_group.usages], ebx
; New field is initialized.
        jmp     .zero_local_items
.padding:
        mov     [ebx+report.size], ecx
        jmp     .zero_local_items
; Create a new collection, nested in the current one.
.collection:
; Actions are quite straightforward:
; allocate, zero-initialize, update parent, if there is one,
; make it current.
        movi    eax, sizeof.collection
        call    Kmalloc
        test    eax, eax
        jz      .memory_error
        push    eax edi
        movi    ecx, sizeof.collection / 4
        xchg    edi, eax
        xor     eax, eax
        rep stosd
        pop     edi eax
        mov     edx, [cur_collection]
        mov     [eax+collection.parent], edx
        lea     ecx, [last_collection]
        test    edx, edx
        jz      .no_parent
        lea     ecx, [edx+collection.last_child]
.no_parent:
        mov     edx, [ecx]
        mov     [ecx], eax
        mov     [edx], eax
        lea     ecx, [eax+collection.first_child]
; In theory, there must be at least one usage.
; In practice, some nested collections don't have any. Use zero in this case.
        mov     edx, [usage_list]
        test    edx, edx
        jz      @f
        mov     edx, [edx+usage_list_item.first_usage]
@@:
        mov     [eax+collection.last_child], ecx
        mov     [eax+collection.type], ebx
        mov     [eax+collection.usage], edx
        mov     [cur_collection], eax
        jmp     .zero_local_items
; Close the current collection.
.end_collection:
; There must be an opened collection.
        mov     eax, [cur_collection]
        test    eax, eax
        jz      .invalid_report
; Make parent collection the current one.
        mov     edx, [eax+collection.parent]
        mov     [cur_collection], edx
; Add field range of the closing collection to field range for nesting collection,
; if there is one.
        test    edx, edx
        jz      .zero_local_items
        push    3       ; for each type: input, output, feature
.update_ranges:
        mov     ecx, [eax+collection.input.last_report]
        test    ecx, ecx
        jz      .no_fields
        mov     [edx+collection.input.last_report], ecx
        mov     ecx, [eax+collection.input.last_field]
        mov     [edx+collection.input.last_field], ecx
        cmp     [edx+collection.input.first_report], 0
        jnz     .no_fields
        mov     ecx, [eax+collection.input.first_report]
        mov     [edx+collection.input.first_report], ecx
        mov     ecx, [eax+collection.input.first_field]
        mov     [edx+collection.input.first_field], ecx
.no_fields:
        add     eax, sizeof.collection_report_set
        add     edx, sizeof.collection_report_set
        dec     dword [esp]
        jnz     .update_ranges
        pop     eax
        jmp     .zero_local_items
; ------------------------------- Global items --------------------------------
.parse_global:
        cmp     eax, .num_global_items
        jae     .item_parsed
        jmp     dword [.global_jumps+eax*4]
; For most global items, just store the value in the current global_items structure.
; Note 1: Usage Page will be used for upper word of Usage[| Minimum|Maximum], so
; shift it in advance.
; Note 2: the HID specification allows both signed and unsigned values for
; logical and physical minimum/maximum, but does not give a method to distinguish.
; Thus, hope that minimum comes first, parse the minimum as signed value always,
; if it is less than zero, assume signed values, otherwise assume unsigned values.
; This covers both common cases Minimum(0)/Maximum(FF) and Minimum(-7F)/Maximum(7F).
; Note 3: zero value for Report ID is forbidden by the HID specification.
; It is quite convenient, we use report_id == 0 for reports without ID.
.usage_page:
        shl     ebx, 16
        mov     [edi+global_items.usage_page], ebx
        jmp     .item_parsed
.logical_minimum:
        mov     [edi+global_items.logical_minimum], ecx
        jmp     .item_parsed
.logical_maximum:
        cmp     [edi+global_items.logical_minimum], 0
        jge     @f
        mov     ebx, ecx
@@:
        mov     [edi+global_items.logical_maximum], ebx
        jmp     .item_parsed
.physical_minimum:
        mov     [edi+global_items.physical_minimum], ecx
        jmp     .item_parsed
.physical_maximum:
        cmp     [edi+global_items.physical_maximum], 0
        jge     @f
        mov     ebx, ecx
@@:
        mov     [edi+global_items.physical_maximum], ebx
        jmp     .item_parsed
.unit_exponent:
        mov     [edi+global_items.unit_exponent], ecx
        jmp     .item_parsed
.unit:
        mov     [edi+global_items.unit], ebx
        jmp     .item_parsed
.report_size:
        mov     [edi+global_items.report_size], ebx
        jmp     .item_parsed
.report_id:
        test    ebx, ebx
        jz      .invalid_report
        cmp     ebx, 0x100
        jae     .invalid_report
        mov     [edi+global_items.report_id], ebx
        jmp     .item_parsed
.report_count:
        mov     [edi+global_items.report_count], ebx
        jmp     .item_parsed
; Two special global items: Push/Pop.
.push:
; For Push, allocate new global_items structure,
; initialize from the current one and make it current.
        movi    eax, sizeof.global_items
        call    Kmalloc
        test    eax, eax
        jz      .memory_error
        push    esi eax
        movi    ecx, sizeof.global_items / 4
        mov     esi, edi
        xchg    eax, edi
        rep movsd
        pop     edi esi
        mov     [edi+global_items.next], eax
        jmp     .item_parsed
.pop:
; For Pop, restore the last global_items structure and free the current one.
        mov     eax, [edi+global_items.next]
        test    eax, eax
        jz      .invalid_report
        push    eax
        xchg    eax, edi
        call    Kfree
        pop     edi
        jmp     .item_parsed
; -------------------------------- Local items --------------------------------
.parse_local:
        cmp     eax, .num_local_items
        jae     .item_parsed
        jmp     dword [.local_jumps+eax*4]
.usage:
; Usage tag.
; If length is 0, 1, 2 bytes, append the global item Usage Page.
        cmp     [cur_item_size], 2
        ja      @f
        or      ebx, [edi+global_items.usage_page]
@@:
; If inside Delimiter(), ignore everything except the first tag.
        cmp     [delimiter_depth], 0
        jz      .usage.write
        inc     [usage_variant]
        cmp     [usage_variant], 1
        jnz     .item_parsed
.usage.write:
; Add new range with start = item data and length = 1.
        mov     [usage_minimum], ebx
        push    1
.new_usage:
        movi    eax, sizeof.usage_list_item
        call    Kmalloc
        pop     edx
        test    eax, eax
        jz      .memory_error
        inc     [num_usage_ranges]
        mov     ecx, [usage_minimum]
        and     [eax+usage_list_item.next], 0
        mov     [eax+usage_list_item.first_usage], ecx
        mov     [eax+usage_list_item.num_usages], edx
        mov     ecx, [usage_tail]
        mov     [usage_tail], eax
        mov     [ecx], eax
        jmp     .item_parsed
.usage_minimum:
; Usage Minimum tag. Just store in the local var.
; If length is 0, 1, 2 bytes, append the global item Usage Page.
        cmp     [cur_item_size], 2
        ja      @f
        or      ebx, [edi+global_items.usage_page]
@@:
        mov     [usage_minimum], ebx
        jmp     .item_parsed
.usage_maximum:
; Usage Maximum tag.
; If length is 0, 1, 2 bytes, append the global item Usage Page.
        cmp     [cur_item_size], 2
        ja      @f
        or      ebx, [edi+global_items.usage_page]
@@:
; Meaningless inside Delimiter().
        cmp     [delimiter_depth], 0
        jnz     .invalid_report
; Add new range with start = saved Usage Minimum and
; length = Usage Maximum - Usage Minimum + 1.
        sub     ebx, [usage_minimum]
        inc     ebx
        push    ebx
        jmp     .new_usage
.delimiter:
; Delimiter tag.
        test    ebx, ebx
        jz      .delimiter.close
; Delimiter(Opened).
; Store that we are inside Delimiter(),
; say a warning that only preferred Usage will be used.
        cmp     [delimiter_depth], 0
        jnz     .invalid_report
        inc     [delimiter_depth]
        push    esi
        mov     esi, delimiter_note
        call    SysMsgBoardStr
        pop     esi
        jmp     .item_parsed
.delimiter.close:
; Delimiter(Closed).
; Store that we are not inside Delimiter() anymore.
        dec     [delimiter_depth]
        js      .invalid_report
        and     [usage_variant], 0
        jmp     .item_parsed
.parse_reserved:
; Ignore reserved items, except that tag 0xFE means long item
; with first data byte = length of additional data,
; second data byte = long item tag. No long items are defined yet,
; so just skip them.
        cmp     eax, 0xF
        jnz     .item_parsed
        cmp     [cur_item_size], 2
        jnz     .item_parsed
        movzx   ecx, bl
        add     esi, ecx
        cmp     esi, [length]
        ja      .invalid_report
.item_parsed:
        cmp     esi, [length]
        jb      .fetch_next_item
.parse_end:
;-------------------------------- End of parsing ------------------------------
; If there are opened collections, it is invalid report.
        cmp     [cur_collection], 0
        jnz     .invalid_report
; There must be at least one input field.
        mov     eax, [calldata]
        add     eax, hid_data.input.first_report
        cmp     [last_reports+0*4], eax
        jz      .invalid_report
; Everything is ok.
        inc     [report_ok]
        jmp     .end
.memory_error:
        mov     esi, nomemory_msg
.end_str:
        call    SysMsgBoardStr
.end:
; Free all global_items structures.
        test    edi, edi
        jz      @f
        push    [edi+global_items.next]
        xchg    eax, edi
        call    Kfree
        pop     edi
        jmp     .end
@@:
; Free the last Usage list, if any.
        mov     eax, [usage_list]
@@:
        test    eax, eax
        jz      @f
        push    [eax+usage_list_item.next]
        call    Kfree
        pop     eax
        jmp     @b
@@:
}

; Assign drivers to top-level HID collections.
; The caller should provide ebx = pointer to hid_data and a local variable
; [has_driver], it will be initialized with 0 if no driver is present.
macro postprocess_descr
{
postprocess_report_label:
; Assign drivers to top-level collections.
; Use mouse driver for Usage(GenericDesktop:Mouse),
; use keyboard driver for Usage(GenericDesktop:Keyboard)
; and Usage(GenericDesktop:Keypad)
; 1. Prepare for the loop: get the pointer to the first collection,
; store that no drivers were assigned yet.
        mov     edi, [ebx+hid_data.first_collection]
if ~HID_DUMP_UNCLAIMED
        mov     [has_driver], 0
end if
.next_collection:
; 2. Test whether there is a collection to test; if no, break from the loop.
        test    edi, edi
        jz      .postprocess_done
; 3. Get pointer to driver callbacks depending on [collection.usage].
; If [collection.usage] is unknown, use default driver if HID_DUMP_UNCLAIMED
; and do not assign a driver otherwise.
        mov     esi, mouse_driver
        cmp     [edi+collection.usage], USAGE_GD_MOUSE
        jz      .has_driver
        mov     esi, keyboard_driver
        cmp     [edi+collection.usage], USAGE_GD_KEYBOARD
        jz      .has_driver
        cmp     [edi+collection.usage], USAGE_GD_KEYPAD
        jz      .has_driver
if HID_DUMP_UNCLAIMED
        mov     esi, default_driver
else
        xor     esi, esi
end if
; 4. If no driver is assigned (possible only if not HID_DUMP_UNCLAIMED),
; go to 7 with driver data = 0;
; other code uses this as a sign that driver callbacks should not be called.
.has_driver:
        xor     eax, eax
if ~HID_DUMP_UNCLAIMED
        test    esi, esi
        jz      .set_driver
end if
; 5. Notify the driver about new device.
        call    [esi+hid_driver_callbacks.add_device]
; 6. If the driver has returned non-zero driver data,
; store that is an assigned driver.
; Otherwise, if HID_DUMP_UNCLAIMED, try to assign the default driver.
if HID_DUMP_UNCLAIMED
        test    eax, eax
        jnz     .set_driver
        mov     esi, default_driver
        call    [esi+hid_driver_callbacks.add_device]
else
        test    eax, eax
        jz      @f
        mov     [has_driver], 1
        jmp     .set_driver
@@:
        xor     esi, esi
end if
.set_driver:
; 7. Store driver data. If a driver is assigned, copy driver callbacks.
        mov     [edi+collection.driver_data], eax
        test    esi, esi
        jz      @f
        push    edi
        lodsd   ; skip hid_driver_callbacks.add_device
        add     edi, collection.callbacks
repeat sizeof.hid_driver_active_callbacks / 4
        movsd
end repeat
        pop     edi
@@:
; 8. Store pointer to the collection in all input reports belonging to it.
; Note that the HID spec requires that reports should not cross top-level collections.
        mov     eax, [edi+collection.input.first_report]
        test    eax, eax
        jz      .reports_processed
.next_report:
        mov     [eax+report.top_level_collection], edi
        cmp     eax, [edi+collection.input.last_report]
        mov     eax, [eax+report.next]
        jnz     .next_report
.reports_processed:
        mov     edi, [edi+collection.next]
        jmp     .next_collection
.postprocess_done:
}

; Cleanup all resources allocated during parse_descr and postprocess_descr.
; Called when the corresponding device is disconnected
; with ebx = pointer to hid_data.
macro hid_cleanup
{
; 1. Notify all assigned drivers about disconnect.
; Loop over all top-level collections and call callbacks.disconnect,
; if a driver is assigned.
        mov     esi, [ebx+hid_data.first_collection]
.notify_drivers:
        test    esi, esi
        jz      .notify_drivers_done
        mov     edi, [esi+collection.driver_data]
        test    edi, edi
        jz      @f
        call    [esi+collection.callbacks.disconnect]
@@:
        mov     esi, [esi+collection.next]
        jmp     .notify_drivers
.notify_drivers_done:
; 2. Free all collections.
        mov     esi, [ebx+hid_data.first_collection]
.free_collections:
        test    esi, esi
        jz      .collections_done
; If a collection has childen, make it forget about them,
; kill all children; after last child is killed, return to
; the collection as a parent; this time, it will appear
; as childless, so it will be killed after children.
        mov     eax, [esi+collection.first_child]
        test    eax, eax
        jz      .no_children
        and     [esi+collection.first_child], 0
        xchg    esi, eax
        jmp     .free_collections
.no_children:
; If a collection has no children (maybe there were no children at all,
; maybe all children were already killed), kill it and proceed either to
; next sibling (if any) or to the parent.
        mov     eax, [esi+collection.next]
        test    eax, eax
        jnz     @f
        mov     eax, [esi+collection.parent]
@@:
        xchg    eax, esi
        call    Kfree
        jmp     .free_collections
.collections_done:
; 3. Free all three report sets.
        push    3
        lea     esi, [ebx+hid_data.input]
; For every report set, loop over all reports,
; for every report free all field groups, then free report itself.
; When all reports in one set have been freed, free also report list table,
; if there is one (reports are numbered).
.report_set_loop:
        mov     edi, [esi+report_set.first_report]
.report_loop:
        test    edi, edi
        jz      .report_done
        mov     eax, [edi+report.first_field]
.field_loop:
        test    eax, eax
        jz      .field_done
        push    [eax+report_field_group.next]
        call    Kfree
        pop     eax
        jmp     .field_loop
.field_done:
        mov     eax, [edi+report.next]
        xchg    eax, edi
        call    Kfree
        jmp     .report_loop
.report_done:
        cmp     [esi+report_set.numbered], 0
        jz      @f
        mov     eax, [esi+report_set.data]
        call    Kfree
@@:
        add     esi, sizeof.report_set
        dec     dword [esp]
        jnz     .report_set_loop
        pop     eax
}

; Helper for parse_input. Extracts value of one field.
; in: esi -> report_field_group
; in: eax = offset in bits from report start
; in: report -> report data
; out: edx = value
; Note: it can read one dword past report data.
macro extract_field_value report
{
        mov     ecx, eax
        shr     eax, 5
        shl     eax, 2
        add     eax, report
        and     ecx, 31
        mov     edx, [eax]
        mov     eax, [eax+4]
        shrd    edx, eax, cl
        mov     ecx, [esi+report_field_group.sign_mask]
        and     ecx, edx
        and     edx, [esi+report_field_group.mask]
        sub     edx, ecx
}

; Local variables for parse_input.
macro parse_input_locals
{
count_inside_group      dd      ?
; Number of fields left in the current field.
field_offset            dd      ?
; Offset of the current field from report start, in bits.
field_range_size                dd      ?
; Size of range with valid values, Logical Maximum - Logical Minimum + 1.
cur_usage               dd      ?
; Pointer to current usage for Variable field groups.
num_values              dd      ?
; Number of values in the current instantiation of Array field group.
values_base             dd      ?
; Pointer to memory allocated for array with current values.
values_prev             dd      ?
; Pointer to memory allocated for array with previous values.
values_cur_ptr          dd      ?
; Pointer to the next value in [values_base] array.
values_end              dd      ?
; End of data in array with current values.
values_prev_ptr         dd      ?
; Pointer to the next value in [values_prev_ptr] array.
values_prev_end         dd      ?
; End of data in array with previous values.
}

; Parse input report. The caller should provide esi = pointer to report,
; local variables parse_input_locals and [buffer] = report data.
macro parse_input
{
; 1. Ignore the report if there is no driver for it.
        mov     ebx, [esi+report.top_level_collection]
        mov     edi, [ebx+collection.driver_data]
        test    edi, edi
        jz      .done
; 2. Notify the driver that a new packet arrived.
        call    [ebx+collection.callbacks.begin_packet]
; Loop over all field groups.
; Report without fields is meaningless, but theoretically possible:
; parse_descr does not create reports of zero size, but
; a report can consist of "padding" fields without usages and have
; no real fields.
        mov     esi, [esi+report.first_field]
        test    esi, esi
        jz      .packet_processed
.field_loop:
; 3. Prepare for group handling: initialize field offset, fields count
; and size of range for valid values.
        mov     eax, [esi+report_field_group.offset]
        mov     [field_offset], eax
        mov     ecx, [esi+report_field_group.count]
        mov     [count_inside_group], ecx
        mov     eax, [esi+report_field_group.logical_maximum]
        inc     eax
        sub     eax, [esi+report_field_group.logical_minimum]
        mov     [field_range_size], eax
; 4. Select handler. Variable and Array groups are handled entirely differently;
; for Variable groups, advance to 5, for Array groups, go to 6.
        test    byte [esi+report_field_group.flags], HID_FIELD_VARIABLE
        jz      .array_field
; 5. Variable groups. They are simple. Loop over all .count fields,
; for every field extract the value and get the next usage,
; if the value is within valid range, call the driver.
        lea     eax, [esi+report_field_group.common_sizeof]
        mov     [cur_usage], eax
.variable_data_loop:
        mov     eax, [field_offset]
        extract_field_value [buffer]    ; -> edx
        mov     ecx, [cur_usage]
        mov     ecx, [ecx]
        call    [ebx+collection.callbacks.input_field]
        add     [cur_usage], 4
        mov     eax, [esi+report_field_group.size]
        add     [field_offset], eax
        dec     [count_inside_group]
        jnz     .variable_data_loop
; Variable group is processed; go to 12.
        jmp     .field_done
.array_field:
; Array groups. They are complicated.
; 6. Array group: extract all values in one array.
; memory was allocated during group creation, use it
; 6a. Prepare: get data pointer, initialize num_values with zero.
        mov     eax, [esi+report_field_group.num_usage_ranges]
        lea     edx, [esi+report_field_group.usages+eax*sizeof.usage_range+4]
        mov     eax, [esi+report_field_group.count]
        mov     [values_cur_ptr], edx
        mov     [values_base], edx
        lea     edx, [edx+ecx*4]
        mov     [values_prev], edx
        mov     [values_prev_ptr], edx
        mov     [num_values], 0
; 6b. Start loop for every field. Note that there must be at least one field,
; parse_descr does not allow .count == 0.
.array_getval_loop:
; 6c. Extract the value of the current field.
        mov     eax, [field_offset]
        extract_field_value [buffer]    ; -> edx
; 6d. Transform the value to the usage with binary search in array of
; usage_ranges. started at [esi+report_field_group.usages]
; having [esi+report_field_group.num_usage_ranges] items.
; Ignore items outside of valid range.
        sub     edx, [esi+report_field_group.logical_minimum]
        cmp     edx, [field_range_size]
        jae     .array_skip_item
; If there are too few usages, use last of them.
        mov     ecx, [esi+report_field_group.num_usage_ranges]  ; upper bound
        xor     eax, eax        ; lower bound
        cmp     edx, [esi+report_field_group.usages+ecx*sizeof.usage_range+usage_range.offset]
        jae     .array_last_usage
; loop invariant: usages[eax].offset <= edx < usages[ecx].offset
.array_find_usage:
        lea     edi, [eax+ecx]
        shr     edi, 1
        cmp     edi, eax
        jz      .array_found_usage_range
        cmp     edx, [esi+report_field_group.usages+edi*sizeof.usage_range+usage_range.offset]
        jae     .update_low
        mov     ecx, edi
        jmp     .array_find_usage
.update_low:
        mov     eax, edi
        jmp     .array_find_usage
.array_last_usage:
        lea     eax, [ecx-1]
        mov     edx, [esi+report_field_group.usages+ecx*sizeof.usage_range+usage_range.offset]
        dec     edx
.array_found_usage_range:
        sub     edx, [esi+report_field_group.usages+eax*sizeof.usage_range+usage_range.offset]
        add     edx, [esi+report_field_group.usages+eax*sizeof.usage_range+usage_range.first_usage]
; 6e. Store the usage, advance data pointer, continue loop started at 6b.
        mov     eax, [values_cur_ptr]
        mov     [eax], edx
        add     [values_cur_ptr], 4
        inc     [num_values]
.array_skip_item:
        mov     eax, [esi+report_field_group.size]
        add     [field_offset], eax
        dec     [count_inside_group]
        jnz     .array_getval_loop
; 7. Array group: ask driver about array overflow.
; If driver says that the array is invalid, stop processing this group
; (in particular, do not update previous values).
        mov     ecx, [num_values]
        test    ecx, ecx
        jz      .duplicates_removed
        mov     edx, [values_base]
        mov     edi, [ebx+collection.driver_data]
        call    [ebx+collection.callbacks.array_overflow?]
        jnc     .field_done
; 8. Array group: sort the array with current values.
        push    esi
        mov     ecx, [num_values]
        mov     edx, [values_base]
        call    sort
        pop     esi
; 9. Array group: remove duplicates.
        cmp     [num_values], 1
        jbe     .duplicates_removed
        mov     eax, [values_base]
        mov     edx, [eax]
        add     eax, 4
        mov     ecx, eax
.duplicates_loop:
        cmp     edx, [eax]
        jz      @f
        mov     edx, [eax]
        mov     [ecx], edx
        add     ecx, 4
@@:
        add     eax, 4
        cmp     eax, [values_cur_ptr]
        jb      .duplicates_loop
        mov     [values_cur_ptr], ecx
        sub     ecx, [values_base]
        shr     ecx, 2
        mov     [num_values], ecx
.duplicates_removed:
; 10. Array group: compare current and previous values,
; call driver for differences.
        mov     edi, [ebx+collection.driver_data]
        mov     eax, [values_cur_ptr]
        mov     [values_end], eax
        mov     eax, [values_base]
        mov     [values_cur_ptr], eax
        mov     eax, [esi+report_field_group.num_values_prev]
        shl     eax, 2
        add     eax, [values_prev]
        mov     [values_prev_end], eax
.find_common:
        mov     eax, [values_cur_ptr]
        cmp     eax, [values_end]
        jae     .cur_done
        mov     ecx, [eax]
        mov     eax, [values_prev_ptr]
        cmp     eax, [values_prev_end]
        jae     .prev_done
        mov     edx, [eax]
        cmp     ecx, edx
        jb      .advance_cur
        ja      .advance_prev
; common item in both arrays; ignore
        add     [values_cur_ptr], 4
        add     [values_prev_ptr], 4
        jmp     .find_common
.advance_cur:
; item is present in current array but not in previous;
; call the driver with value = 1
        add     [values_cur_ptr], 4
        mov     edx, 1
        call    [ebx+collection.callbacks.input_field]
        jmp     .find_common
.advance_prev:
; item is present in previous array but not in current;
; call the driver with value = 0
        add     [values_prev_ptr], 4
        mov     ecx, edx
        xor     edx, edx
        call    [ebx+collection.callbacks.input_field]
        jmp     .find_common
.prev_done:
; for all items which are left in current array
; call the driver with value = 1
        mov     eax, [values_cur_ptr]
@@:
        add     [values_cur_ptr], 4
        mov     ecx, [eax]
        mov     edx, 1
        call    [ebx+collection.callbacks.input_field]
        mov     eax, [values_cur_ptr]
        cmp     eax, [values_end]
        jb      @b
        jmp     .copy_array
.cur_done:
; for all items which are left in previous array
; call the driver with value = 0
        mov     eax, [values_prev_ptr]
        add     [values_prev_ptr], 4
        cmp     eax, [values_prev_end]
        jae     @f
        mov     ecx, [eax]
        xor     edx, edx
        call    [ebx+collection.callbacks.input_field]
        jmp     .cur_done
@@:
.copy_array:
; 11. Array group: copy current values to previous values.
        push    esi edi
        mov     ecx, [num_values]
        mov     [esi+report_field_group.num_values_prev], ecx
        mov     esi, [values_base]
        mov     edi, [values_prev]
        rep movsd
        pop     edi esi
; 12. Field group is processed. Repeat with the next group, if any.
.field_done:
        mov     esi, [esi+report_field_group.next]
        test    esi, esi
        jnz     .field_loop
.packet_processed:
; 13. Packet is processed, notify the driver.
        call    [ebx+collection.callbacks.end_packet]
}
