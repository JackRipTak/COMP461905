#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <elf.h>
#include <unistd.h> //for getpagesize
#include <fcntl.h>
#include<assert.h>

#include <sys/mman.h>

#include "Link.h"
#include "LoaderInternal.h"

#define ALIGN_DOWN(base, size) ((base) & -((__typeof__(base))(size)))
#define ALIGN_UP(base, size) ALIGN_DOWN((base) + (size)-1, (size))

static const char *sys_path[] = {
    "/usr/lib/x86_64-linux-gnu/",
    "/lib/x86_64-linux-gnu/",
    ""
};

static const char *fake_so[] = {
    "libc.so.6",
    "ld-linux.so.2",
    ""
};

static void setup_hash(LinkMap *l)
{
    uint32_t *hash;

    /* borrowed from dl-lookup.c:_dl_setup_hash */
    Elf32_Word *hash32 = (Elf32_Word *)l->dynInfo[DT_GNU_HASH_NEW]->d_un.d_ptr;
    l->l_nbuckets = *hash32++;
    Elf32_Word symbias = *hash32++;
    Elf32_Word bitmask_nwords = *hash32++;

    l->l_gnu_bitmask_idxbits = bitmask_nwords - 1;
    l->l_gnu_shift = *hash32++;

    l->l_gnu_bitmask = (Elf64_Addr *)hash32;
    hash32 += 64 / 32 * bitmask_nwords;

    l->l_gnu_buckets = hash32;
    hash32 += l->l_nbuckets;
    l->l_gnu_chain_zero = hash32 - symbias;
}

static void fill_info(LinkMap *lib)
{
    Elf64_Dyn *dyn = lib->dyn;
    Elf64_Dyn **dyn_info = lib->dynInfo;

    while (dyn->d_tag != DT_NULL)
    {
        if ((Elf64_Xword)dyn->d_tag < DT_NUM)
            dyn_info[dyn->d_tag] = dyn;
        else if ((Elf64_Xword)dyn->d_tag == DT_RELACOUNT)
            dyn_info[DT_RELACOUNT_NEW] = dyn;
        else if ((Elf64_Xword)dyn->d_tag == DT_GNU_HASH)
            dyn_info[DT_GNU_HASH_NEW] = dyn;
        ++dyn;
    }
    #define rebase(tag)                             \
        do                                          \
        {                                           \
            if (dyn_info[tag])                          \
                dyn_info[tag]->d_un.d_ptr += lib->addr; \
        } while (0)
    rebase(DT_SYMTAB);
    rebase(DT_STRTAB);
    rebase(DT_RELA);
    rebase(DT_JMPREL);
    rebase(DT_GNU_HASH_NEW); //DT_GNU_HASH
    rebase(DT_PLTGOT);
    rebase(DT_INIT);
    rebase(DT_INIT_ARRAY);
}

void *MapLibrary(const char *libpath)
{
    /*=
     * hint:
     * 
     * lib = malloc(sizeof(LinkMap));
     * 
     * foreach segment:
     * mmap(start_addr, segment_length, segment_prot, MAP_FILE | ..., library_fd, 
     *      segment_offset);
     * 
     * lib -> addr = ...;
     * lib -> dyn = ...;
     * 
     * fill_info(lib);
     * setup_hash(lib);
     * 
     * return lib;
    */
   
    /* Your code here */
    //printf("%d %d\n",PT_LOAD, PT_DYNAMIC);
      int fd = open(libpath, O_RDONLY);
    Elf64_Ehdr *ehd = mmap(NULL, sizeof(Elf64_Ehdr), PROT_READ, MAP_PRIVATE, fd, 0);
    LinkMap *lib = malloc(sizeof(LinkMap));
    lib -> addr = 0;
    Elf64_Phdr *seg = (Elf64_Phdr *) ((char *)ehd + ehd -> e_phoff );
    for (int i = 0; i < ehd->e_phnum; i++) {
        Elf64_Phdr *cur = &seg[i];
        if(cur->p_type == PT_LOAD){
            int prot = 0;
            prot |= (cur->p_flags & PF_R) ? PROT_READ : 0;
            prot |= (cur->p_flags & PF_W) ? PROT_WRITE : 0;
            prot |= (cur->p_flags & PF_X) ? PROT_EXEC : 0;
            
            void * sp= ALIGN_DOWN(cur->p_vaddr+lib->addr,getpagesize());
            void *tmp;
            if(!lib->addr)
                tmp = mmap(NULL, 1000000, prot, MAP_FILE | MAP_PRIVATE  , fd, ALIGN_DOWN(cur->p_offset, getpagesize()));
            else
                tmp = mmap(ALIGN_DOWN(cur->p_vaddr+lib->addr,getpagesize()), ALIGN_UP(cur->p_memsz, getpagesize()), prot, MAP_FILE | MAP_PRIVATE |MAP_FIXED , fd, ALIGN_DOWN(cur->p_offset, getpagesize())); 
            if(!lib->addr) lib->addr = tmp;
            
        }
        else if(cur->p_type == PT_DYNAMIC){
            lib->dyn=(lib -> addr + cur->p_vaddr);
        }
    }
    fill_info(lib);
    setup_hash(lib);
    return lib;
}
