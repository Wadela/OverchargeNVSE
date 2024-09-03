#pragma once
#include "ShellNVSE.h"

struct Event {
    SInt32 priority;
    Script* script;
    AuxVector filters;

    Event(SInt32 priority, Script* script);
    Event(SInt32 priority, Script* script, AuxVector filters);

    bool CompareFilters(const AuxVector& otherFilters) const;      //Use when dispatching
    static AuxVector EvaluateEventArg(int num_args, ...);
    static AuxVector EvaluateEventArgAux(int num_args, ...);
    
};

struct EventHandler {

    std::vector<Event> handlers;

    void AddEvent(const Event& event);
    void RemoveEvent(const Event& event);

    //template<typename... Args>
    //void DispatchEvent(Args&&... args);
    //void DispatchEvent(TESForm* hello, TESForm* no);

    template<typename... Args>
    void DispatchEvent(Args&&... args) {

        AuxVector filter{ std::forward<Args>(args)... };
        for (auto& eventHandler : handlers) {
            if (eventHandler.CompareFilters(filter)) {
                g_scriptInterface->CallFunction(eventHandler.script, nullptr, nullptr, nullptr, filter.size(), args...);
            }
        }

    }
};

extern EventHandler onEquipAltEvent;

extern EventHandler onInstanceDeconstructEvent;
extern EventHandler onInstanceReconstructEvent;

extern EventHandler onAttachWeapModEvent;
extern EventHandler onAttachWeapModReconstructEvent;

extern EventHandler onDetachWeapModEvent;
extern EventHandler onDetachWeapModDeconstructEvent;

extern EventHandler onAnimationStart;