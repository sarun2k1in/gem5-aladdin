/*
 * Copyright (c) 2004-2005 The Regents of The University of Michigan
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

#ifndef __CPU_O3_ROB_HH__
#define __CPU_O3_ROB_HH__

#include <string>
#include <utility>
#include <vector>

/**
 * ROB class.  The ROB is largely what drives squashing.
 */
template <class Impl>
class ROB
{
  protected:
    typedef TheISA::RegIndex RegIndex;
  public:
    //Typedefs from the Impl.
    typedef typename Impl::FullCPU FullCPU;
    typedef typename Impl::DynInstPtr DynInstPtr;

    typedef std::pair<RegIndex, PhysRegIndex> UnmapInfo;
    typedef typename std::list<DynInstPtr>::iterator InstIt;

    /** Possible ROB statuses. */
    enum Status {
        Running,
        Idle,
        ROBSquashing,
        DcacheMissStall,
        DcacheMissComplete
    };

    /** SMT ROB Sharing Policy */
    enum ROBPolicy{
        Dynamic,
        Partitioned,
        Threshold
    };

  private:
    /** Per-thread ROB status. */
    Status robStatus[Impl::MaxThreads];

    /** ROB resource sharing policy for SMT mode. */
    ROBPolicy robPolicy;

  public:
    /** ROB constructor.
     *  @param _numEntries      Number of entries in ROB.
     *  @param _squashWidth     Number of instructions that can be squashed in a
     *                          single cycle.
     *  @param _smtROBPolicy    ROB Partitioning Scheme for SMT.
     *  @param _smtROBThreshold Max Resources(by %) a thread can have in the ROB.
     *  @param _numThreads      The number of active threads.
     */
    ROB(unsigned _numEntries, unsigned _squashWidth, std::string smtROBPolicy,
        unsigned _smtROBThreshold, unsigned _numThreads);

    std::string name() const;

    /** Function to set the CPU pointer, necessary due to which object the ROB
     *  is created within.
     *  @param cpu_ptr Pointer to the implementation specific full CPU object.
     */
    void setCPU(FullCPU *cpu_ptr);

    /** Sets pointer to the list of active threads.
     *  @param at_ptr Pointer to the list of active threads.
     */
    void setActiveThreads(std::list<unsigned>* at_ptr);

    /** Function to insert an instruction into the ROB. Note that whatever
     *  calls this function must ensure that there is enough space within the
     *  ROB for the new instruction.
     *  @param inst The instruction being inserted into the ROB.
     */
    void insertInst(DynInstPtr &inst);

    /** Returns pointer to the head instruction within the ROB.  There is
     *  no guarantee as to the return value if the ROB is empty.
     *  @retval Pointer to the DynInst that is at the head of the ROB.
     */
    DynInstPtr readHeadInst();

    /** Returns a pointer to the head instruction of a specific thread within
     *  the ROB.
     *  @return Pointer to the DynInst that is at the head of the ROB.
     */
    DynInstPtr readHeadInst(unsigned tid);

    /** Returns pointer to the tail instruction within the ROB.  There is
     *  no guarantee as to the return value if the ROB is empty.
     *  @retval Pointer to the DynInst that is at the tail of the ROB.
     */
    DynInstPtr readTailInst();

    /** Returns a pointer to the tail instruction of a specific thread within
     *  the ROB.
     *  @return Pointer to the DynInst that is at the tail of the ROB.
     */
    DynInstPtr readTailInst(unsigned tid);

    /** Retires the head instruction, removing it from the ROB. */
    void retireHead();

    /** Retires the head instruction of a specific thread, removing it from the
     *  ROB.
     */
    void retireHead(unsigned tid);

    /** Is the oldest instruction across all threads ready. */
    bool isHeadReady();

    /** Is the oldest instruction across a particular thread ready. */
    bool isHeadReady(unsigned tid);

    /** Is there any commitable head instruction across all threads ready. */
    bool canCommit();

    /** Re-adjust ROB partitioning. */
    void resetEntries();

    /** Number of entries needed For 'num_threads' amount of threads. */
    int entryAmount(int num_threads);

    /** Returns the number of total free entries in the ROB. */
    unsigned numFreeEntries();

    /** Returns the number of free entries in a specific ROB paritition. */
    unsigned numFreeEntries(unsigned tid);

    /** Returns the maximum number of entries for a specific thread. */
    unsigned getMaxEntries(unsigned tid)
    { return maxEntries[tid]; }

    /** Returns the number of entries being used by a specific thread. */
    unsigned getThreadEntries(unsigned tid)
    { return threadEntries[tid]; }

    /** Returns if the ROB is full. */
    bool isFull()
    { return numInstsInROB == numEntries; }

    /** Returns if a specific thread's partition is full. */
    bool isFull(unsigned tid)
    { return threadEntries[tid] == numEntries; }

    /** Returns if the ROB is empty. */
    bool isEmpty()
    { return numInstsInROB == 0; }

    /** Returns if a specific thread's partition is empty. */
    bool isEmpty(unsigned tid)
    { return threadEntries[tid] == 0; }

    /** Executes the squash, marking squashed instructions. */
    void doSquash(unsigned tid);

    /** Squashes all instructions younger than the given sequence number for
     *  the specific thread.
     */
    void squash(InstSeqNum squash_num, unsigned tid);

    /** Updates the head instruction with the new oldest instruction. */
    void updateHead();

    /** Updates the tail instruction with the new youngest instruction. */
    void updateTail();

    /** Reads the PC of the oldest head instruction. */
    uint64_t readHeadPC();

    /** Reads the PC of the head instruction of a specific thread. */
    uint64_t readHeadPC(unsigned tid);

    /** Reads the next PC of the oldest head instruction. */
    uint64_t readHeadNextPC();

    /** Reads the next PC of the head instruction of a specific thread. */
    uint64_t readHeadNextPC(unsigned tid);

    /** Reads the sequence number of the oldest head instruction. */
    InstSeqNum readHeadSeqNum();

    /** Reads the sequence number of the head instruction of a specific thread.
     */
    InstSeqNum readHeadSeqNum(unsigned tid);

    /** Reads the PC of the youngest tail instruction. */
    uint64_t readTailPC();

    /** Reads the PC of the tail instruction of a specific thread. */
    uint64_t readTailPC(unsigned tid);

    /** Reads the sequence number of the youngest tail instruction. */
    InstSeqNum readTailSeqNum();

    /** Reads the sequence number of tail instruction of a specific thread. */
    InstSeqNum readTailSeqNum(unsigned tid);

    /** Checks if the ROB is still in the process of squashing instructions.
     *  @retval Whether or not the ROB is done squashing.
     */
    bool isDoneSquashing(unsigned tid) const
    { return doneSquashing[tid]; }

    /** Checks if the ROB is still in the process of squashing instructions for
     *  any thread.
     */
    bool isDoneSquashing();

    /** This is more of a debugging function than anything.  Use
     *  numInstsInROB to get the instructions in the ROB unless you are
     *  double checking that variable.
     */
    int countInsts();

    /** This is more of a debugging function than anything.  Use
     *  threadEntries to get the instructions in the ROB unless you are
     *  double checking that variable.
     */
    int countInsts(unsigned tid);

  private:
    /** Pointer to the CPU. */
    FullCPU *cpu;

    /** Active Threads in CPU */
    std::list<unsigned>* activeThreads;

    /** Number of instructions in the ROB. */
    unsigned numEntries;

    /** Entries Per Thread */
    unsigned threadEntries[Impl::MaxThreads];

    /** Max Insts a Thread Can Have in the ROB */
    unsigned maxEntries[Impl::MaxThreads];

    /** ROB List of Instructions */
    std::list<DynInstPtr> instList[Impl::MaxThreads];

    /** Number of instructions that can be squashed in a single cycle. */
    unsigned squashWidth;

  public:
    /** Iterator pointing to the instruction which is the last instruction
     *  in the ROB.  This may at times be invalid (ie when the ROB is empty),
     *  however it should never be incorrect.
     */
    InstIt tail;

    /** Iterator pointing to the instruction which is the first instruction in
     *  in the ROB*/
    InstIt head;

  private:
    /** Iterator used for walking through the list of instructions when
     *  squashing.  Used so that there is persistent state between cycles;
     *  when squashing, the instructions are marked as squashed but not
     *  immediately removed, meaning the tail iterator remains the same before
     *  and after a squash.
     *  This will always be set to cpu->instList.end() if it is invalid.
     */
    InstIt squashIt[Impl::MaxThreads];

  public:
    /** Number of instructions in the ROB. */
    int numInstsInROB;

    DynInstPtr dummyInst;

  private:
    /** The sequence number of the squashed instruction. */
    InstSeqNum squashedSeqNum;

    /** Is the ROB done squashing. */
    bool doneSquashing[Impl::MaxThreads];

    /** Number of active threads. */
    unsigned numThreads;
};

#endif //__CPU_O3_ROB_HH__
