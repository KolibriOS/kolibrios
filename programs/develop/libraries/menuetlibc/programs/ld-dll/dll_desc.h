typedef struct {
 char dll_name[1];
} dll_req_entry_t;

typedef struct {
 dll_req_entry_t req_list[0];
} dll_req_t;

#define DLL_REQ_LIST_NAME	"__DLL_REQUIRE_LIST__"
