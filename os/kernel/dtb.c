#include "dtb.h"
#include "types.h"

#define DTB_MAGIC_LE DTB_BYTESWAP32(0xd00dfeed)

#define NULL ((void *)0)

int strncmp(const char*, const char*, uint);

int strcmp_nodename(const char *pathpart, const char *nodename)
{
    while (!(*pathpart == '\0' || *nodename == '\0')) {
        if (*pathpart != *nodename) {
            break;
        }

        pathpart++;
        nodename++;
    }

    if (*pathpart == '\0' && *nodename == '\0') {
        return 0;
    }

    if (*pathpart == '\0' && *nodename == '@') {
        return 0;
    }

    return 1;
}

uint32 *next_token(uint32 *token)
{
    if (*token == DTB_BEGIN_NODE) {
        token++;

        char *tokenchar = (char *)token;
        while (*tokenchar != '\0') {
            tokenchar++;
        }

        if ((uint64_t)tokenchar % 4 != 0) {
            tokenchar += 4 - (uint64_t)tokenchar % 4;
        }
        token = (uint32_t *)tokenchar;

        while (*token == 0) {
            token++;
        }
    } else if (*token == DTB_END_NODE) {
        token++;
    } else if (*token == DTB_PROP) {
        uint32 len = DTB_BYTESWAP32(*(token + 1));
        if (len % sizeof(uint32) == 0) {
            token += len / sizeof(uint32) + 3;
        } else {
            uint32 len_rounding = 4 - (len % 4);
            token += (len + len_rounding) / sizeof(uint32) + 3;
        }
    } else if (*token == DTB_NOP) {
        token++;
    }

    return token;
}


dtb *dtb_fromptr(void *ptr)
{
    dtb *devicetree = (dtb *)ptr;

    if (devicetree == NULL) {
        return NULL;
    }

    if (devicetree->magic != DTB_MAGIC_LE) {
        return NULL;
    }

    return devicetree;
}

dtb_node dtb_find(dtb *devicetree, const char *path)
{
    uint32 *struct_block = (uint32 *)((uint8_t *)devicetree + DTB_BYTESWAP32(devicetree->off_dt_struct));

    // The first block from the struct node should be a DTB_BEGIN_NODE as it should be refering to
    // the root node of the device tree. If that is not the case we return null to signal an error.
    if (*struct_block != DTB_BEGIN_NODE) {
        return NULL;
    }

    if (strncmp(path, "/", 2) == 0) {
        return (dtb_node)struct_block;
    }

    // Always needs to be an absolute path.
    if (path[0] != '/') {
        return NULL;
    }

    char parsed_path[10][256] = {0};
    uint64 parsed_i = 0;
    uint64 path_depth = 0;
    int i = 1;
    while (path[i] != '\0') {
        if (path[i] == '/') {
            path_depth++;
            parsed_i = 0;
        } else {
            parsed_path[path_depth][parsed_i] = path[i];
            parsed_i++;
        }

        i++;
    }

    uint32 *token = struct_block + 1;
    uint32 parsing_depth = 0;
    while (*token != DTB_END) {
        if (*token == DTB_BEGIN_NODE) {
            token++;
            if (strcmp_nodename(parsed_path[parsing_depth], (char *)token) == 0) {
                if (parsing_depth == path_depth) {
                    return (dtb_node)token - 1;
                }

                parsing_depth++;
            } else {
                token = dtb_next_sibling(token - 1);
                if (token == NULL) {
                    return NULL;
                }
                token--;
            }
        } else if (*token == DTB_PROP) {
            uint32 len = DTB_BYTESWAP32(*(token + 1));
            token += len / sizeof(uint32) + 2;
        }

        token++;
    }

    return NULL;
}

char *dtb_property_name(dtb *devicetree, dtb_node node)
{
    char *strings = (char *)devicetree + DTB_BYTESWAP32(devicetree->off_dt_strings);
    uint32_t stroff = DTB_BYTESWAP32(*(node + 2));
    return strings + stroff;
}

uint32_t dtb_property_uint32(dtb_property prop)
{
    return DTB_BYTESWAP32(*(prop + 3));
}

uint64_t dtb_property_uint64(dtb_property prop)
{
    return DTB_BYTESWAP64(*(uint64_t *)(prop + 3));
}

char *dtb_property_string(dtb_property prop)
{
    return (char *)(prop + 3);
}

char *dtb_property_array(dtb_property prop)
{
    return (void *)(prop + 3);
}

dtb_property dtb_first_property(dtb_node node)
{
    uint32 *token = next_token(node);
    while (*token != DTB_PROP) {
        if (*token == DTB_END || *token == DTB_BEGIN_NODE || *token == DTB_END_NODE) {
            return NULL;
        }

        token = next_token(token);
    }

    return token;
}

dtb_property dtb_next_property(dtb_property prop)
{
    uint32 *token = next_token(prop);
    while (*token != DTB_PROP) {
        if (*token == DTB_END || *token == DTB_BEGIN_NODE || *token == DTB_END_NODE) {
            return NULL;
        }

        token = next_token(token);
    }

    return token;
}

dtb_node dtb_first_child(dtb_node node)
{
    uint32 *token = next_token(node);
    while (*token != DTB_BEGIN_NODE) {
        if (*token == DTB_END || *token == DTB_END_NODE) {
            return NULL;
        }

        token = next_token(token);
    }
    return token;
}

dtb_node dtb_next_sibling(dtb_node node)
{
    int depth = 0;
    while (*node != DTB_END) {
        if (*node == DTB_BEGIN_NODE) {
            node++;
            depth++;
        } else if (*node == DTB_PROP) {
            uint32 len = DTB_BYTESWAP32(*(node + 1));
            int len_rounding = 4 - len % 4;
            if (len % 4 == 0) {
                node += len / sizeof(uint32) + 2;
            } else {
                node += (len + len_rounding) / sizeof(uint32) + 2;
            }
        } else if (*node == DTB_END_NODE) {
            depth--;
            if (depth <= 0) {
                node++;
                break;
            }
        }
        node++;
    }

    while (*node != DTB_BEGIN_NODE) {
        if (*node == DTB_END) {
            break;
        }

        if (*node == DTB_END_NODE) {
            return NULL;
        }

        node++;
    }

    return node;
}

dtb_rsvmap_entry *dtb_first_rsvmap_entry(dtb *devicetree)
{
    uint8_t *dtb_ptr = (uint8_t *)devicetree;
    dtb_rsvmap_entry *entry = (dtb_rsvmap_entry *)(dtb_ptr + DTB_BYTESWAP32(devicetree->off_mem_rsvmap));

    if (entry->address == 0 && entry->size == 0) {
        return NULL;
    }

    return entry;
}

dtb_rsvmap_entry *dtb_next_rsvmap_entry(dtb_rsvmap_entry *entry)
{
    entry++;

    if (entry->address == 0 && entry->size == 0) {
        return NULL;
    }

    return entry;
}
