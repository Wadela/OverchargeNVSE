#pragma once
#include "EventHandlers.h"
#include <ParamInfosNVSE.h>

EventHandler onEquipAltEvent;

EventHandler onInstanceReconstructEvent;
EventHandler onInstanceDeconstructEvent;

EventHandler onAttachWeapModEvent;
EventHandler onAttachWeapModReconstructEvent;

EventHandler onDetachWeapModEvent;
EventHandler onDetachWeapModDeconstructEvent;

EventHandler onAnimationStart;

Event::Event(SInt32 priority, Script* script) : priority(priority), script(script) {}

Event::Event(SInt32 priority, Script* script, AuxVector filters)
    : priority(priority), script(script), filters(filters) {}

bool Event::CompareFilters(const AuxVector& pfilters) const {      //Use when dispatching

    if (filters.size() != pfilters.size()) {
        return false;
    }

    for (size_t i = 0; i < filters.size(); ++i) {

        if (filters[i].type != -1 && filters[i].type != pfilters[i].type) {
            return false;
        }

        switch (filters[i].type) {
        case kRetnType_Default:
            if (filters[i].num != pfilters[i].num) {
                return false;
            }
            break;
        case kRetnType_Form:
            if (filters[i].refID != pfilters[i].refID) {
                return false;
            }
            break;
        case kRetnType_String:
            if (strcmp(filters[i].str, pfilters[i].str) != 0) {
                return false;
            }
            break;
        case kRetnType_Array:
            if (filters[i].NVSEArray != pfilters[i].NVSEArray) {
                return false;
            }
            break;
        }
    }

    return true;

}

AuxVector Event::EvaluateEventArg(int num_args, ...) {
    va_list args;
    va_start(args, num_args);
    AuxVector filter(num_args);

    for (int i = 0; i < num_args; i++) {
        UInt32 index = va_arg(args, UInt32);
        if (index != 0) {                           //Filter skipped.
            if (index <= num_args) {

                PluginScriptToken* arg = va_arg(args, PluginScriptToken*);

                if (arg) {
                    switch (arg->GetType()) {
                    case kTokenType_Number:
                    case kTokenType_NumericVar:
                        filter.SetValue((index - 1), arg->GetFloat());
                        break;
                    case kTokenType_Form:
                    case kTokenType_RefVar:
                        filter.SetValue((index - 1), arg->GetFormID());
                        break;
                    case kTokenType_String:
                    case kTokenType_StringVar:
                        filter.SetValue((index - 1), arg->GetString());
                        break;
                    case kTokenType_Array:
                    case kTokenType_ArrayVar:
                        filter.SetValue((index - 1), arg->GetArrayVar());
                        break;
                    }
                }

            }
            else {
                va_arg(args, void*);    //Skip, index out of scope.
            }
        }
        else {
            break;
        }
    }

    va_end(args);
    return filter;

}

AuxVector Event::EvaluateEventArgAux(int num_args, ...) {

    va_list args;
    va_start(args, num_args);
    AuxVector filter(num_args);

    for (int i = 0; i < num_args; i++) {
        UInt32 index = va_arg(args, UInt32);
        if (index != 0) {                           //Filter skipped.
            if (index <= num_args) {

                filter[index - 1] = va_arg(args, AuxValue);

            }
            else {
                va_arg(args, void*);    //Skip, priority out of scope.
            }
        }
        else {
            break;
        }
    }

    va_end(args);
    return filter;

}

void EventHandler::AddEvent(const Event& event) {

    auto it = handlers.begin();
    for (it; it != handlers.end(); ++it) {
        if (it->CompareFilters(event.filters) && it->script == event.script) {
            it->priority = event.priority;
            it->filters = event.filters;
            return;
        }
    }

    if (it == handlers.end()) {
        auto insertPos = std::lower_bound(handlers.begin(), handlers.end(), event, [](const Event& a, const Event& b) {
            return a.priority < b.priority;
            });
        handlers.insert(insertPos, event);
    }
    else {
        
    }

}

void EventHandler::RemoveEvent(const Event& event) {
    auto it = handlers.begin();
    for (it; it != handlers.end(); ++it) {
        if (it->CompareFilters(event.filters) && it->script == event.script) {
            handlers.erase(it);
            return;
        }
    }

}

//template<typename... Args>
//void EventHandler::DispatchEvent(Args&&... args) {

   // std::vector<void*> filter{ std::forward<Args>(args)... };
    //for (auto& eventHandler : handlers) {
        //if (eventHandler.CompareFilters(filter)) {
            //g_scriptInterface->CallFunction(eventHandler.script, nullptr, nullptr, nullptr, filter.size(), args...);
       // }
    //}

//}