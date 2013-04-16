// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/input/input.h"

#include <string>

#include "base/lazy_instance.h"
#include "base/string_util.h"
#include "base/strings/string_number_conversions.h"
#include "base/values.h"
#include "chrome/browser/extensions/extension_function_registry.h"
#include "chrome/browser/ui/top_level_widget.h"
#include "chrome/common/chrome_notification_types.h"
#include "content/public/browser/browser_thread.h"
#include "ui/base/events/event.h"
#include "ui/base/events/key_identifier_conversion.h"

#if defined(USE_ASH) && defined(USE_AURA)
#include "ash/shell.h"
#include "ui/aura/root_window.h"
#endif

namespace extensions {

namespace {

// Keys.
const char kType[] = "type";
const char kKeyIdentifier[] = "keyIdentifier";
const char kAlt[] = "altKey";
const char kCtrl[] = "ctrlKey";
const char kMeta[] = "metaKey";
const char kShift[] = "shiftKey";
const char kKeyDown[] = "keydown";
const char kKeyUp[] = "keyup";

// Errors.
const char kUnknownEventTypeError[] = "Unknown event type.";
const char kUnknownOrUnsupportedKeyIdentiferError[] = "Unknown or unsupported "
    "key identifier.";
const char kUnsupportedModifier[] = "Unsupported modifier.";
const char kNoValidRecipientError[] = "No valid recipient for event.";
const char kKeyEventUnprocessedError[] = "Event was not handled.";

ui::EventType GetTypeFromString(const std::string& type) {
  if (type == kKeyDown) {
    return ui::ET_KEY_PRESSED;
  } else if (type == kKeyUp) {
    return ui::ET_KEY_RELEASED;
  }
  return ui::ET_UNKNOWN;
}

// Converts a hex string "U+NNNN" to uint16. Returns 0 on error.
uint16 UnicodeIdentifierStringToInt(const std::string& key_identifier) {
  int character = 0;
  if ((key_identifier.length() == 6) &&
      (key_identifier.substr(0, 2) == "U+") &&
      (key_identifier.substr(2).find_first_not_of("0123456789abcdefABCDEF") ==
       std::string::npos)) {
    const bool result =
        base::HexStringToInt(key_identifier.substr(2), &character);
    DCHECK(result) << key_identifier;
  }
  return character;
}

}  // namespace

bool SendKeyboardEventInputFunction::RunImpl() {
  DictionaryValue* args;
  EXTENSION_FUNCTION_VALIDATE(args_->GetDictionary(0, &args));

  std::string type_name;
  EXTENSION_FUNCTION_VALIDATE(args->GetString(kType, &type_name));
  ui::EventType type = GetTypeFromString(type_name);
  if (type == ui::ET_UNKNOWN) {
    error_ = kUnknownEventTypeError;
    return false;
  }

  std::string identifier;
  EXTENSION_FUNCTION_VALIDATE(args->GetString(kKeyIdentifier, &identifier));
  TrimWhitespaceASCII(identifier, TRIM_ALL, &identifier);

  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));
  const ui::KeyEvent& prototype_event =
      ui::KeyEventFromKeyIdentifier(identifier);
  uint16 character = 0;
  if (prototype_event.key_code() == ui::VKEY_UNKNOWN) {
    // Check if |identifier| is "U+NNNN" format.
    character = UnicodeIdentifierStringToInt(identifier);
    if (!character) {
      error_ = kUnknownOrUnsupportedKeyIdentiferError;
      return false;
    }
  }

  bool flag = false;
  int flags = 0;
  if (prototype_event.key_code() != ui::VKEY_UNKNOWN)
    flags = prototype_event.flags();
  flags |= (args->GetBoolean(kAlt, &flag) && flag) ? ui::EF_ALT_DOWN : 0;
  flags |= (args->GetBoolean(kCtrl, &flag) && flag) ? ui::EF_CONTROL_DOWN : 0;
  flags |= (args->GetBoolean(kShift, &flag) && flag) ? ui::EF_SHIFT_DOWN : 0;
  if (args->GetBoolean(kMeta, &flag) && flag) {
    // Views does not have a Meta event flag, so return an error for now.
    error_ = kUnsupportedModifier;
    return false;
  }

  ui::KeyEvent event(type,
                     prototype_event.key_code(),
                     flags,
                     prototype_event.is_char());
  if (character) {
    event.set_character(character);
    event.set_unmodified_character(character);
  }

#if defined(USE_ASH) && defined(USE_AURA)
  ash::Shell::GetActiveRootWindow()->AsRootWindowHostDelegate()->OnHostKeyEvent(
      &event);
  return true;
#else
  return false;
#endif
}

InputAPI::InputAPI(Profile* profile) {
  ExtensionFunctionRegistry* registry =
      ExtensionFunctionRegistry::GetInstance();
  registry->RegisterFunction<SendKeyboardEventInputFunction>();
}

InputAPI::~InputAPI() {
}

static base::LazyInstance<ProfileKeyedAPIFactory<InputAPI> >
g_factory = LAZY_INSTANCE_INITIALIZER;

// static
ProfileKeyedAPIFactory<InputAPI>* InputAPI::GetFactoryInstance() {
  return &g_factory.Get();
}

}  // namespace extensions
