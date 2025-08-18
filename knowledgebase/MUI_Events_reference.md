# MUIA_ Attributes for DoMethod (MUI Event Handling)

This document lists common MUIA_ (Magic User Interface Attribute) constants that can be used with DoMethod for event handling and notifications in Amiga MUI applications. It also provides typical use cases and code examples.

---

## What are MUIA_ Attributes?
MUIA_ attributes are used to get or set properties of MUI objects, and are often used in DoMethod calls to set up notifications (event handling) between objects.

---

## Common MUIA_ Attributes for DoMethod

### 1. MUIA_Pressed
- **Description:** Indicates if a button is pressed (TRUE) or released (FALSE).
- **Use case:** Detect when a button is clicked.
- **Example:**
  ```c
  DoMethod(bt1, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, 1);
  // When bt1 is released, send return ID 1 to app
  ```

### 2. MUIA_Selected
- **Description:** Indicates if a checkmark, radio button, or similar is selected.
- **Use case:** Detect when a checkmark or radio button is toggled.
- **Example:**
  ```c
  DoMethod(check, MUIM_Notify, MUIA_Selected, TRUE, app, 2, MUIM_Application_ReturnID, 2);
  // When check is selected, send return ID 2 to app
  ```

### 3. MUIA_Window_CloseRequest
- **Description:** Indicates if the window close gadget was pressed.
- **Use case:** Handle window close events.
- **Example:**
  ```c
  DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
  ```

### 4. MUIA_ShowMe
- **Description:** Controls or queries the visibility of an object.
- **Use case:** Show or hide objects in response to events.
- **Example:**
  ```c
  DoMethod(bt1, MUIM_Notify, MUIA_Pressed, FALSE, someObject, 3, MUIM_Set, MUIA_ShowMe, TRUE);
  // When bt1 is released, show someObject
  ```

### 5. MUIA_String_Contents
- **Description:** The contents of a StringObject (text field).
- **Use case:** React to changes in a text field.
- **Example:**
  ```c
  DoMethod(str, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, app, 2, MUIM_Application_ReturnID, 3);
  // On every change, send return ID 3 to app
  ```

### 6. MUIA_Cycle_Active / MUIA_Radio_Active
- **Description:** The active entry in a cycle or radio gadget.
- **Use case:** Detect when the user changes selection.
- **Example:**
  ```c
  DoMethod(cycle, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, app, 2, MUIM_Application_ReturnID, 4);
  ```

---

## General Notification Pattern

```c
DoMethod(sourceObj, MUIM_Notify, attribute, value, targetObj, numParams, method, ...);
```
- **sourceObj:** The object to watch for changes.
- **attribute:** The MUIA_ attribute to monitor.
- **value:** The value to trigger on (e.g., TRUE, FALSE, MUIV_EveryTime).
- **targetObj:** The object to notify.
- **method:** The method to call (e.g., MUIM_Application_ReturnID, MUIM_Set).

---

## More Attributes
- MUIA_Window_Open
- MUIA_Weight
- MUIA_Active
- MUIA_Disabled
- MUIA_String_MaxLen
- MUIA_Frame
- ...and many more (see the MUI SDK documentation for a full list)

---

## References
- [MUI SDK Documentation](https://muidev.de/)
- See also: `MUI_macros_reference.md` in this workspace for macro usage

---

This file is a quick reference for common event-related MUIA_ attributes. For a full list, consult the MUI SDK headers and documentation.
