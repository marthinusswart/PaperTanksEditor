# MUI Custom Class Creation and Dispatcher Implementation Analysis

Based on the analysis of MUI Custom Class source code patterns from the Amiga MUI ecosystem, this document summarizes the different approaches for creating MUI Custom Classes and implementing their dispatchers.

## Overview

MUI (Magic User Interface) Custom Classes are object-oriented components that extend the MUI toolkit functionality on Amiga systems. They follow the BOOPSI (Basic Object Oriented Programming System for Intuition) architecture and provide a standardized way to create reusable GUI components.

## MUI Custom Class Creation Methods

### 1. Standard MUI Custom Class Structure

The most common approach involves creating a custom class that inherits from an existing MUI class:

```c
// Basic class creation pattern
struct MUI_CustomClass *CreateCustomClass(void)
{
    return MUI_CreateCustomClass(
        NULL,                    // No base custom class
        MUIC_Area,              // Base MUI class (Area, Group, etc.)
        NULL,                   // No user data
        sizeof(struct Data),    // Instance data size
        ENTRY(MyDispatcher)     // Dispatcher function
    );
}
```

### 2. Subclassing Existing MUI Classes

Classes can inherit from various base classes depending on functionality:

- **MUIC_Area**: For basic drawable objects
- **MUIC_Group**: For container objects that hold other objects  
- **MUIC_Gadget**: For interactive elements
- **MUIC_Window**: For window-based classes
- **MUIC_Application**: For application-level classes

### 3. Class Registration and Library Structure

For distributing as .mcc (MUI Custom Class) libraries:

```c
// Library initialization
struct Library *MCC_InitLib(void)
{
    // Initialize class
    if ((MyClass = CreateCustomClass()))
    {
        return (struct Library *)MyClass->mcc_Class;
    }
    return NULL;
}

// Library cleanup
void MCC_ExitLib(void)
{
    if (MyClass)
    {
        MUI_DeleteCustomClass(MyClass);
        MyClass = NULL;
    }
}
```

## Dispatcher Implementation Patterns

The dispatcher is the central message handling function that processes all method calls to the custom class. There are several implementation patterns:

### 1. Simple Switch-Based Dispatcher

The most straightforward approach using a switch statement:

```c
DISPATCHER(MyDispatcher)
{
    switch (msg->MethodID)
    {
        case OM_NEW:
            return DoSuperNew(cl, obj,
                TAG_MORE, (ULONG)msg,
                TAG_END);
        
        case OM_DISPOSE:
            return DoSuperMethodA(cl, obj, msg);
        
        case OM_SET:
            return mSet(cl, obj, (struct opSet *)msg);
        
        case OM_GET:
            return mGet(cl, obj, (struct opGet *)msg);
        
        case MUIM_Draw:
            return mDraw(cl, obj, (struct MUIP_Draw *)msg);
        
        default:
            return DoSuperMethodA(cl, obj, msg);
    }
}
```

### 2. Function Table-Based Dispatcher

Using function pointers for better organization and performance:

```c
// Method function table
static const struct {
    ULONG methodID;
    APTR function;
} MethodTable[] = {
    { OM_NEW,     mNew },
    { OM_DISPOSE, mDispose },
    { OM_SET,     mSet },
    { OM_GET,     mGet },
    { MUIM_Draw,  mDraw },
    { ~0UL,       NULL }
};

DISPATCHER(MyDispatcher)
{
    int i;
    
    for (i = 0; MethodTable[i].function; i++)
    {
        if (MethodTable[i].methodID == msg->MethodID)
        {
            return ((ULONG (*)(struct IClass *, Object *, Msg))
                    MethodTable[i].function)(cl, obj, msg);
        }
    }
    
    return DoSuperMethodA(cl, obj, msg);
}
```

### 3. Hybrid Dispatcher with Performance Optimization

Combining both approaches for optimal performance:

```c
DISPATCHER(MyDispatcher)
{
    // Handle most common methods with switch for speed
    switch (msg->MethodID)
    {
        case OM_NEW:     return mNew(cl, obj, msg);
        case OM_DISPOSE: return mDispose(cl, obj, msg);
        case OM_SET:     return mSet(cl, obj, msg);
        case OM_GET:     return mGet(cl, obj, msg);
        case MUIM_Draw:  return mDraw(cl, obj, msg);
        case MUIM_Setup: return mSetup(cl, obj, msg);
        
        default:
            // Handle less common methods
            if (msg->MethodID >= MUIM_MyClass_First && 
                msg->MethodID <= MUIM_MyClass_Last)
            {
                return HandleCustomMethods(cl, obj, msg);
            }
            return DoSuperMethodA(cl, obj, msg);
    }
}
```

## Essential Method Implementations

### Object Lifecycle Methods

```c
// Object creation
static ULONG mNew(struct IClass *cl, Object *obj, struct opSet *msg)
{
    if ((obj = (Object *)DoSuperMethodA(cl, obj, (Msg)msg)))
    {
        struct Data *data = INST_DATA(cl, obj);
        
        // Initialize instance data
        data->field1 = 0;
        data->field2 = NULL;
        
        // Process initial attributes
        mSet(cl, obj, msg);
    }
    
    return (ULONG)obj;
}

// Object destruction  
static ULONG mDispose(struct IClass *cl, Object *obj, Msg msg)
{
    struct Data *data = INST_DATA(cl, obj);
    
    // Cleanup instance data
    if (data->field2)
        FreeVec(data->field2);
    
    return DoSuperMethodA(cl, obj, msg);
}
```

### Attribute Handling

```c
// Set attributes
static ULONG mSet(struct IClass *cl, Object *obj, struct opSet *msg)
{
    struct Data *data = INST_DATA(cl, obj);
    struct TagItem *ti;
    ULONG result = 0;
    
    while ((ti = NextTagItem(&msg->ops_AttrList)))
    {
        switch (ti->ti_Tag)
        {
            case MUIA_MyClass_Value:
                data->value = ti->ti_Data;
                MUI_Redraw(obj, MADF_DRAWOBJECT);
                result = TRUE;
                break;
        }
    }
    
    return result | DoSuperMethodA(cl, obj, (Msg)msg);
}

// Get attributes
static ULONG mGet(struct IClass *cl, Object *obj, struct opGet *msg)
{
    struct Data *data = INST_DATA(cl, obj);
    
    switch (msg->opg_AttrID)
    {
        case MUIA_MyClass_Value:
            *msg->opg_Storage = data->value;
            return TRUE;
    }
    
    return DoSuperMethodA(cl, obj, (Msg)msg);
}
```

## Advanced Dispatcher Features

### Custom Method Ranges

```c
#define MUIM_MyClass_DoSomething    (TAG_USER | 0x12340001)
#define MUIM_MyClass_Calculate      (TAG_USER | 0x12340002)
#define MUIM_MyClass_First          MUIM_MyClass_DoSomething
#define MUIM_MyClass_Last           MUIM_MyClass_Calculate

// Custom method implementation
static ULONG mDoSomething(struct IClass *cl, Object *obj, 
                         struct MUIP_MyClass_DoSomething *msg)
{
    struct Data *data = INST_DATA(cl, obj);
    
    // Custom method logic
    return TRUE;
}
```

### Event Handling and Notifications

```c
// Input event handling
static ULONG mHandleEvent(struct IClass *cl, Object *obj,
                         struct MUIP_HandleEvent *msg)
{
    if (msg->imsg->Class == IDCMP_MOUSEBUTTONS)
    {
        if (msg->imsg->Code == SELECTDOWN)
        {
            // Send notification
            set(obj, MUIA_Pressed, TRUE);
            DoMethod(_app(obj), MUIM_Application_ReturnID, 
                    MUIV_Application_ReturnID_Quit);
        }
    }
    
    return DoSuperMethodA(cl, obj, (Msg)msg);
}
```

## Best Practices

1. **Always call superclass methods**: Use `DoSuperMethodA()` for unhandled methods
2. **Proper memory management**: Clean up in `mDispose()`
3. **Thread safety**: Use appropriate locking mechanisms if needed
4. **Error handling**: Check return values and handle edge cases
5. **Documentation**: Comment method behavior and attribute meanings
6. **Performance**: Optimize frequently called methods like `MUIM_Draw`

## Conclusion

MUI Custom Classes provide a powerful and flexible framework for creating reusable GUI components. The dispatcher pattern allows for clean separation of concerns while maintaining high performance through various optimization techniques. Proper implementation of the lifecycle methods and attribute handling ensures robust and maintainable custom classes.

Exactly right — you've nailed it. 🎯

---

## ✅ Subclasses Always Require `NewObject()`

When you create a **custom subclass** using `MUI_CreateCustomClass()`, you get a `struct MUI_CustomClass *`, which contains:

- `mcc_Class`: the actual BOOPSI class pointer
- Your dispatcher
- Your instance data size

But `MUI_NewObject()` is designed only for **built-in MUI classes** like `MUIC_Text`, `MUIC_Group`, etc. It looks up the class by name and handles internal registration. It doesn’t know about your custom subclass — so it can’t instantiate it.

---

## 🔍 Why `MUI_NewObject()` Fails for Subclasses

```c
Object *obj = MUI_NewObject(PTEImagePanelClass, ...); // ❌ Invalid
```

This fails because `MUI_NewObject()` expects a class name like `"Text"` or `"Group"`, not a pointer to a `MUI_CustomClass`.

---

## ✅ Correct Usage for Subclasses

```c
Object *obj = NewObject(PTEImagePanelClass->mcc_Class, NULL, TAG_END);
```

This works — but only if:
- `PTEImagePanelClass` is non-NULL
- `PTEImagePanelClass->mcc_Class` is valid
- You’ve provided a dispatcher

---

## 🧩 Summary

| Class Type        | Use `MUI_NewObject()` | Use `NewObject()` |
|-------------------|-----------------------|-------------------|
| Native MUI class  | ✅ Yes                | ✅ Yes (if ready) |
| Custom subclass   | ❌ No                 | ✅ Required       |

---

Want help writing a macro like `NEW_SUBCLASS_OBJECT(class, tags...)` that wraps the safety checks and makes your code cleaner? Could save you from future crashes.
