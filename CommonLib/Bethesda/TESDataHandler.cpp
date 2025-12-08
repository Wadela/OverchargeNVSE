#include "TESDataHandler.hpp"

bool TESDataHandler::DoAddForm(TESForm* apForm) {
	return ThisCall(0x4603B0, this, apForm);
}

TESForm* TESDataHandler::CreateFormFromID(uint8_t aucID) {
	return CdeclCall<TESForm*>(0x465110, aucID);
}

TESForm* TESForm::CloneForm(bool persist) const {
	TESForm* result = TESDataHandler::CreateFormFromID(eTypeID);
	if (result) {
		result->Copy(this);
		TESDataHandler::GetSingleton()->DoAddForm(result);
	}

	return result;
}