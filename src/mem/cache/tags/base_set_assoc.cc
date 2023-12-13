/*
 * Copyright (c) 2012-2014 ARM Limited
 * All rights reserved.
 *
 * The license below extends only to copyright in the software and shall
 * not be construed as granting a license to any other intellectual
 * property including but not limited to intellectual property relating
 * to a hardware implementation of the functionality of the software
 * licensed hereunder.  You may use the software subject to the license
 * terms below provided that you ensure that this notice is replicated
 * unmodified and in its entirety in all distributions of the software,
 * modified or unmodified, in source code or in binary form.
 *
 * Copyright (c) 2003-2005,2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file
 * Definitions of a conventional tag store.
 */

#include "mem/cache/tags/base_set_assoc.hh"

#include <string>

#include "base/intmath.hh"
#include "debug/kalabhya.hh"
#include "sim/cur_tick.hh"

namespace gem5
{

BaseSetAssoc::BaseSetAssoc(const Params &p)
    :BaseTags(p), allocAssoc(p.assoc), blks(p.size / p.block_size),
     sequentialAccess(p.sequential_access),
     replacementPolicy(p.replacement_policy)
{
    // There must be a indexing policy
    fatal_if(!p.indexing_policy, "An indexing policy is required");

    // Check parameters
    if (blkSize < 4 || !isPowerOf2(blkSize)) {
        fatal("Block size must be at least 4 and a power of 2");
    }
}

void
BaseSetAssoc::tagsInit()
{
    // Initialize all blocks
    for (unsigned blk_index = 0; blk_index < numBlocks; blk_index++) {
        // Locate next cache block
        CacheBlk* blk = &blks[blk_index];

        // Link block to indexing policy
        indexingPolicy->setEntry(blk, blk_index);

        // Associate a data chunk to the block
        blk->data = &dataBlks[blkSize*blk_index];

        // Associate a replacement data entry to the block
        blk->replacementData = replacementPolicy->instantiateEntry();

        //added by kalabhya
        blk->setBlkSize(blkSize);
    }
}

void
BaseSetAssoc::invalidate(CacheBlk *blk)
{
    BaseTags::invalidate(blk);

    // Decrease the number of tags in use
    stats.tagsInUse--;

    // Invalidate replacement data
    replacementPolicy->invalidate(blk->replacementData);
    //kalabhya
   blk->lastTouch = Tick(0);
}

void
BaseSetAssoc::moveBlock(CacheBlk *src_blk, CacheBlk *dest_blk)
{
    BaseTags::moveBlock(src_blk, dest_blk);

    // Since the blocks were using different replacement data pointers,
    // we must touch the replacement data of the new entry, and invalidate
    // the one that is being moved.
    replacementPolicy->invalidate(src_blk->replacementData);
    src_blk->lastTouch = Tick(0);
    replacementPolicy->reset(dest_blk->replacementData);
    dest_blk->lastTouch = curTick();
}

CacheBlk* BaseSetAssoc::findVictimVariableSegment(Addr addr,
                        const bool is_secure,
                        const std::size_t size,
                        std::vector<CacheBlk*>& evict_blks,
                        bool update_expansion)
{
    DPRINTF(kalabhya, "inside base assoc findVictim\n");
    // Get possible entries to be victimized
    const std::vector<ReplaceableEntry*> entries =
        indexingPolicy->getPossibleEntries(addr);

    Addr tag = extractTag(addr);
    unsigned set_size = 0;
    CacheBlk* victim = nullptr;
    CacheBlk* curr_blk = nullptr;
    std::vector<ReplaceableEntry*> valid_entries;

    for (const auto& entry : entries) {
        CacheBlk* entry_block = static_cast<CacheBlk*>(entry);
        DPRINTF(kalabhya, "static_cast to cacheblk first for loop\n");
        if (entry_block->isValid()) {
            if (entry_block->matchTag(tag, is_secure) && update_expansion) {
                curr_blk = entry_block;
                victim = curr_blk;
            }
            else {
                DPRINTF(kalabhya, "acessed isValid first for loop\n");
                valid_entries.push_back(entry);
                set_size += entry_block->_size;
                DPRINTF(kalabhya, "accessed size first for loop\n");
            }
        }
        else {
            victim = entry_block;
            evict_blks.push_back(victim);
        }

    }

    unsigned max_set_size = (allocAssoc/2)*blkSize*CHAR_BIT;
    DPRINTF(kalabhya, "max_set_size computed\n");

    int size_diff = size - (max_set_size - set_size);
    DPRINTF(kalabhya, "first size_diff computed\n");

    if (size_diff > 0 || !victim) {
        DPRINTF(kalabhya, "entering first LRU findVictim\n");
        victim = static_cast<CacheBlk*>(
            replacementPolicy->getVictim(valid_entries));
        DPRINTF(kalabhya, "exited first LRU findVictim\n");
        evict_blks.push_back(victim);
    }

    size_diff -= victim->_size;
    DPRINTF(kalabhya, "second size_diff computed\n");

    if (size_diff > 0) {
        std::vector<ReplaceableEntry*> new_valid_entries;
        DPRINTF(kalabhya, "second size_diff check\n");
        for (const auto& entry : valid_entries) {
            CacheBlk* entry_block = static_cast<CacheBlk*>(entry);
            DPRINTF(kalabhya, "static_cast to cacheblk second for loop\n");
            if (entry_block->_size >= size_diff) {
                DPRINTF(kalabhya, "acessed size second for loop\n");
                new_valid_entries.push_back(entry);
            }
        }

        victim = static_cast<CacheBlk*>(
            replacementPolicy->getVictim(new_valid_entries));
        DPRINTF(kalabhya, "exited second LRU findVictim\n");
        evict_blks.push_back(victim);
    }
    DPRINTF(kalabhya, "exit\n");

    if (update_expansion) {
        return curr_blk;
    }
    return victim;
}

} // namespace gem5
