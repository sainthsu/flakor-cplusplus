/****************************************************************************
Copyright (c) 2013 flakor.org

http://www.flakor.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

/**
 @file
 based on Chipmunk cpArray.
 ccArray is a faster alternative to NSMutableArray, it does pretty much the
 same thing (stores NSObjects and retains/releases them appropriately). It's
 faster because:
 - it uses a plain C interface so it doesn't incur Objective-c messaging overhead
 - it assumes you know what you're doing, so it doesn't spend time on safety checks
 (index out of bounds, required capacity etc.)
 - comparisons are done using pointer equality instead of isEqual

 There are 2 kind of functions:
 - ccArray functions that manipulates objective-c objects (retain and release are performed)
 - ccCArray functions that manipulates values like if they were standard C structures (no retain/release is performed)
 */

#ifndef _FK_CARRAY_H_
#define _FK_CARRAY_H_

#include "macros.h"
#include "base/lang/Object.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>

FLAKOR_NS_BEGIN

#define FK_INVALID_INDEX 0xffffffff

// Easy integration
#define FK_ARRAYDATA_FOREACH(__array__, __object__)															\
__object__=__array__->arr[0]; for(unsigned int i=0, num=__array__->num; i<num; i++, __object__=__array__->arr[i])	\


typedef struct _fkArray {
	unsigned int num, max;
	Object** arr;
} fkArray;

/** Allocates and initializes a new array with specified capacity */
fkArray* fkArrayNew(unsigned int capacity);

/** Frees array after removing all remaining objects. Silently ignores nil arr. */
void fkArrayFree(fkArray*& arr);

/** Doubles array capacity */
void fkArrayDoubleCapacity(fkArray *arr);

/** Increases array capacity such that max >= num + extra. */
void fkArrayEnsureExtraCapacity(fkArray *arr, unsigned int extra);

/** shrinks the array so the memory footprint corresponds with the number of items */
void fkArrayShrink(fkArray *arr);

/** Returns index of first occurrence of object, NSNotFound if object not found. */
unsigned int fkArrayGetIndexOfObject(fkArray *arr, Object* object);

/** Returns a Boolean value that indicates whether object is present in array. */
bool fkArrayContainsObject(fkArray *arr, Object* object);

/** Appends an object. Behavior undefined if array doesn't have enough capacity. */
void fkArrayAppendObject(fkArray *arr, Object* object);

/** Appends an object. Capacity of arr is increased if needed. */
void fkArrayAppendObjectWithResize(fkArray *arr, Object* object);

/** Appends objects from plusArr to arr. 
 Behavior undefined if arr doesn't have enough capacity. */
void fkArrayAppendArray(fkArray *arr, fkArray *plusArr);

/** Appends objects from plusArr to arr. Capacity of arr is increased if needed. */
void fkArrayAppendArrayWithResize(fkArray *arr, fkArray *plusArr);

/** Inserts an object at index */
void fkArrayInsertObjectAtIndex(fkArray *arr, Object* object, unsigned int index);

/** Swaps two objects */
void fkArraySwapObjectsAtIndexes(fkArray *arr, unsigned int index1, unsigned int index2);

/** Removes all objects from arr */
void fkArrayRemoveAllObjects(fkArray *arr);

/** Removes object at specified index and pushes back all subsequent objects.
 Behavior undefined if index outside [0, num-1]. */
void fkArrayRemoveObjectAtIndex(fkArray *arr, unsigned int index, bool bReleaseObj = true);

/** Removes object at specified index and fills the gap with the last object,
 thereby avoiding the need to push back subsequent objects.
 Behavior undefined if index outside [0, num-1]. */
void fkArrayFastRemoveObjectAtIndex(fkArray *arr, unsigned int index);

void fkArrayFastRemoveObject(fkArray *arr, Object* object);

/** Searches for the first occurrence of object and removes it. If object is not
 found the function has no effect. */
void fkArrayRemoveObject(fkArray *arr, Object* object, bool bReleaseObj = true);

/** Removes from arr all objects in minusArr. For each object in minusArr, the
 first matching instance in arr will be removed. */
void fkArrayRemoveArray(fkArray *arr, fkArray *minusArr);

/** Removes from arr all objects in minusArr. For each object in minusArr, all
 matching instances in arr will be removed. */
void fkArrayFullRemoveArray(fkArray *arr, fkArray *minusArr);

// #pragma mark -
// #pragma mark fkCArray for Values (c structures)

typedef struct _fkCArray {
    unsigned int num, max;
    void** arr;
} fkCArray;

/** Allocates and initializes a new C array with specified capacity */
fkCArray* fkCArrayNew(unsigned int capacity);

/** Frees C array after removing all remaining values. Silently ignores nil arr. */
void fkCArrayFree(fkCArray *arr);

/** Doubles C array capacity */
void fkCArrayDoubleCapacity(fkCArray *arr);

/** Increases array capacity such that max >= num + extra. */
void fkCArrayEnsureExtraCapacity(fkCArray *arr, unsigned int extra);

/** Returns index of first occurrence of value, NSNotFound if value not found. */
unsigned int fkCArrayGetIndexOfValue(fkCArray *arr, void* value);

/** Returns a Boolean value that indicates whether value is present in the C array. */
bool fkCArrayContainsValue(fkCArray *arr, void* value);

/** Inserts a value at a certain position. Behavior undefined if array doesn't have enough capacity */
void fkCArrayInsertValueAtIndex( fkCArray *arr, void* value, unsigned int index);

/** Appends an value. Behavior undefined if array doesn't have enough capacity. */
void fkCArrayAppendValue(fkCArray *arr, void* value);

/** Appends an value. Capacity of arr is increased if needed. */
void fkCArrayAppendValueWithResize(fkCArray *arr, void* value);

/** Appends values from plusArr to arr. Behavior undefined if arr doesn't have
 enough capacity. */
void fkCArrayAppendArray(fkCArray *arr, fkCArray *plusArr);

/** Appends values from plusArr to arr. Capacity of arr is increased if needed. */
void fkCArrayAppendArrayWithResize(fkCArray *arr, fkCArray *plusArr);

/** Removes all values from arr */
void fkCArrayRemoveAllValues(fkCArray *arr);

/** Removes value at specified index and pushes back all subsequent values.
 Behavior undefined if index outside [0, num-1].
 @since v0.99.4
 */
void fkCArrayRemoveValueAtIndex(fkCArray *arr, unsigned int index);

/** Removes value at specified index and fills the gap with the last value,
 thereby avoiding the need to push back subsequent values.
 Behavior undefined if index outside [0, num-1].
 @since v0.99.4
 */
void fkCArrayFastRemoveValueAtIndex(fkCArray *arr, unsigned int index);

/** Searches for the first occurrence of value and removes it. If value is not found the function has no effect.
 @since v0.99.4
 */
void fkCArrayRemoveValue(fkCArray *arr, void* value);

/** Removes from arr all values in minusArr. For each Value in minusArr, the first matching instance in arr will be removed.
 @since v0.99.4
 */
void fkCArrayRemoveArray(fkCArray *arr, fkCArray *minusArr);

/** Removes from arr all values in minusArr. For each value in minusArr, all matching instances in arr will be removed.
 @since v0.99.4
 */
void fkCArrayFullRemoveArray(fkCArray *arr, fkCArray *minusArr);

FLAKOR_NS_END
	
#endif // FK_ARRAY_H_
