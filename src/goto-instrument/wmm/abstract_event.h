/*******************************************************************\

Module: abstract events

Author: Vincent Nimal

Date: 2012

\*******************************************************************/

/// \file
/// abstract events

#ifndef CPROVER_GOTO_INSTRUMENT_WMM_ABSTRACT_EVENT_H
#define CPROVER_GOTO_INSTRUMENT_WMM_ABSTRACT_EVENT_H

#include <util/source_location.h>
#include <util/graph.h>

#include "wmm.h"

class abstract_eventt:public graph_nodet<empty_edget>
{
protected:
  bool unsafe_pair_lwfence_param(const abstract_eventt &next,
    memory_modelt model, bool lwsync_met) const;

public:
  /* for now, both fence functions and asm fences accepted */
  enum class operationt { Write, Read, Fence, Lwfence, ASMfence };

  operationt operation = operationt::Write;
  unsigned thread = 0;
  irep_idt variable;
  unsigned id = 0;
  source_locationt source_location;
  bool local = false;

  // for ASMfence
  bool WRfence = false;
  bool WWfence = false;
  bool RRfence = false;
  bool RWfence = false;
  bool WWcumul = false;
  bool RWcumul = false;
  bool RRcumul = false;

  abstract_eventt()
  {
  }

  abstract_eventt(
    operationt _op, unsigned _th, irep_idt _var,
    unsigned _id, source_locationt _loc, bool _local)
    :operation(_op), thread(_th), variable(_var), id(_id),
    source_location(_loc), local(_local)
  {
  }

  abstract_eventt(
    operationt _op,
    unsigned _th,
    irep_idt _var,
    unsigned _id,
    source_locationt _loc,
    bool _local,
    bool WRf,
    bool WWf,
    bool RRf,
    bool RWf,
    bool WWc,
    bool RWc,
    bool RRc):
    operation(_op),
    thread(_th),
    variable(_var),
    id(_id),
    source_location(_loc),
    local(_local),
    WRfence(RWf),
    WWfence(WWf),
    RRfence(RRf),
    RWfence(WRf),
    WWcumul(WWc),
    RWcumul(RWc),
    RRcumul(RRc)
  {
  }

  /* post declaration (through graph) -- doesn't copy */
  void operator()(const abstract_eventt &other)
  {
    operation=other.operation;
    thread=other.thread;
    variable=other.variable;
    id=other.id;
    source_location=other.source_location;
    local=other.local;
  }

  bool operator==(const abstract_eventt &other) const
  {
    return (id == other.id);
  }

  bool operator<(const abstract_eventt &other) const
  {
    return (id < other.id);
  }

  bool is_fence() const
  {
    return operation==operationt::Fence ||
           operation==operationt::Lwfence ||
           operation==operationt::ASMfence;
  }

  /* checks the safety of the pair locally (i.e., w/o taking fences
     or dependencies into account -- use is_unsafe on the whole
     critical cycle for this) */
  bool unsafe_pair(const abstract_eventt &next, memory_modelt model) const
  {
    return unsafe_pair_lwfence_param(next, model, false);
  }

  bool unsafe_pair_lwfence(
    const abstract_eventt &next,
    memory_modelt model) const
  {
    return unsafe_pair_lwfence_param(next, model, true);
  }

  bool unsafe_pair_asm(
    const abstract_eventt &next,
    memory_modelt model,
    unsigned char met) const;

  std::string get_operation() const
  {
    switch(operation)
    {
      case operationt::Write: return "W";
      case operationt::Read: return "R";
      case operationt::Fence: return "F";
      case operationt::Lwfence: return "f";
      case operationt::ASMfence: return "asm:";
    }
    assert(false);
    return "?";
  }

  bool is_corresponding_fence(const abstract_eventt &first,
    const abstract_eventt &second) const
  {
    return
      (WRfence &&
       first.operation==operationt::Write &&
       second.operation==operationt::Read) ||
      ((WWfence || WWcumul) &&
       first.operation==operationt::Write &&
       second.operation==operationt::Write) ||
      ((RWfence || RWcumul) &&
       first.operation==operationt::Read &&
       second.operation==operationt::Write) ||
      ((RRfence || RRcumul) &&
       first.operation==operationt::Read &&
       second.operation==operationt::Read);
  }

  bool is_direct() const { return WWfence || WRfence || RRfence || RWfence; }
  bool is_cumul() const { return WWcumul || RWcumul || RRcumul; }

  unsigned char fence_value() const
  {
    unsigned char value = WRfence + 2*WWfence + 4*RRfence + 8*RWfence
      + 16*WWcumul + 32*RWcumul + 64*RRcumul;
    return value;
  }
};

inline std::ostream &operator<<(
  std::ostream &s,
  const abstract_eventt &e)
{
  return s << e.get_operation() << e.variable;
}

#endif // CPROVER_GOTO_INSTRUMENT_WMM_ABSTRACT_EVENT_H
