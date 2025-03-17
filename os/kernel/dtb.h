#ifndef _DTB_H
#define _DTB_H

#include <stdint.h>
#include <stddef.h>
typedef uint8_t  dtb_u8;
typedef uint32_t dtb_u32;
typedef uint64_t dtb_u64;
#define DTB_NULL NULL

#define DTB_BYTESWAP32(num) ((((num)>>24)&0xff) | (((num)<<8)&0xff0000) | \
                        (((num)>>8)&0xff00) | (((num)<<24)&0xff000000))

#define DTB_BYTESWAP64(num) (DTB_BYTESWAP32(num) << 32 | DTB_BYTESWAP32((num) >> 32))

#define DTB_BEGIN_NODE DTB_BYTESWAP32((dtb_u32)0x1)
#define DTB_END_NODE   DTB_BYTESWAP32((dtb_u32)0x2)
#define DTB_PROP       DTB_BYTESWAP32((dtb_u32)0x3)
#define DTB_NOP        DTB_BYTESWAP32((dtb_u32)0x4)
#define DTB_END        DTB_BYTESWAP32((dtb_u32)0x9)

typedef struct
{
    dtb_u32 magic;
    dtb_u32 totalsize;
    dtb_u32 off_dt_struct;
    dtb_u32 off_dt_strings;
    dtb_u32 off_mem_rsvmap;
    dtb_u32 version;
    dtb_u32 last_comp_version;
    dtb_u32 boot_cpuid_phys;
    dtb_u32 size_dt_strings;
    dtb_u32 size_dt_struct;
} dtb;

typedef struct __attribute__((packed))
{
    dtb_u64 address;
    dtb_u64 size;
} dtb_rsvmap_entry;

typedef dtb_u32* dtb_node;
typedef dtb_u32* dtb_property;

dtb *dtb_fromptr(void *ptr);

dtb_node dtb_find(dtb *devicetree, const char *path);

dtb_node dtb_find_next(dtb_node node, char *name);

dtb_node dtb_find_by_phandle(dtb *devicetree, dtb_u32 phandle);

#define dtb_node_name(node) (char *)((dtb_u32 *)node+1)

char *dtb_property_name(dtb *devicetree, dtb_node node);

dtb_u32 dtb_property_uint32(dtb_property prop);

dtb_u64 dtb_property_uint64(dtb_property prop);

char *dtb_property_string(dtb_property prop);

char *dtb_property_array(dtb_property prop);

dtb_u32 dtb_property_length(dtb_property prop);

char *dtb_property_next_string(char *str);

#define dtb_foreach_stringlist(prop, str) for (char *str = dtb_property_string(prop); str < (char *)dtb_next_token(prop); str = dtb_property_next_string(str))

#define dtb_foreach_property(node, name) for (dtb_property name = dtb_first_property(node); name != NULL; name = dtb_next_property(name))

dtb_property dtb_first_property(dtb_property prop);

dtb_property dtb_next_property(dtb_property prop);

dtb_node dtb_first_child(dtb_node node);

dtb_node dtb_next_sibling(dtb_node node);

#define dtb_foreach_child(node, name) for (dtb_node name = dtb_first_child(node); name != NULL; name = dtb_next_sibling(name))

#define dtb_foreach_reg(prop, addr_cells, size_cells, reg) for (dtb_u32 *reg = prop+3; reg < dtb_next_token(prop); reg += addr_cells + size_cells)

dtb_u64 dtb_reg_start(dtb_u32 *reg, dtb_u8 addr_cells);

dtb_u64 dtb_reg_size(dtb_u32 *reg, dtb_u8 addr_cells, dtb_u8 size_cells);

dtb_rsvmap_entry *dtb_first_rsvmap_entry(dtb *devicetree);

dtb_rsvmap_entry *dtb_next_rsvmap_entry(dtb_rsvmap_entry *entry);

#define dtb_foreach_rsvmap_entry(dtb, entry) for (dtb_rsvmap_entry *entry = dtb_first_rsvmap_entry(dtb); entry != NULL; entry = dtb_next_rsvmap_entry(entry))

dtb_u32 *dtb_next_token(dtb_u32 *token);

#endif // _DTB_H
