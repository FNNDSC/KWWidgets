/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkVector.h,v $
  Language:  C++
  Date:      $Date: 2002-02-21 14:48:02 $
  Version:   $Revision: 1.2 $


Copyright (c) 1993-2001 Ken Martin, Will Schroeder, Bill Lorensen 
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither name of Ken Martin, Will Schroeder, or Bill Lorensen nor the names
   of any contributors may be used to endorse or promote products derived
   from this software without specific prior written permission.

 * Modified source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
// .NAME vtkVector - a dynamic vector

#include "vtkContainer.h"

template <class T>
class VTK_COMMON_EXPORT vtkVector : public vtkContainer
{
  // Description:
  // This is a prototype for a compare function. It has to
  // return true if objects are the same and false if not.
  typedef int (*CompareFunction)(T item1, T item2);

public:
  static vtkVector<T> *New() { return new vtkVector<T>(); }  
  
  // Description:
  // Append an Item to the end of the vector
  unsigned long AppendItem(T a) 
    {
      if ((this->NumberOfItems + 1) >= this->Size)
        {
        if (!this->Size)
          {
          this->Size = 2;
          }
        T *newArray = new T [this->Size*2];
        unsigned int i;
        for (i = 0; i < this->NumberOfItems; ++i)
          {
          newArray[i] = this->Array[i];
          }
        this->Size = this->Size*2;
        if (this->Array)
          {
          delete [] this->Array;
          }
        this->Array = newArray;
        }
      this->Array[this->NumberOfItems] = a;
      this->NumberOfItems++;
      return (this->NumberOfItems - 1);
    }
  
  // Description:
  // Remove an Item from the vector
  unsigned long RemoveItem(unsigned long id) 
    {
      if (id >= this->NumberOfItems)
        {
        return 0;
        }
      unsigned int i;
      this->NumberOfItems--;
      for (i = id; i < this->NumberOfItems; ++i)
        {
        this->Array[i] = this->Array[i+1];
        }
      return 1;
    }
  
  // Description:
  // Return an item that was previously added to this vector. 
  int GetItem(unsigned long id, T& ret) 
    {
      if (id < this->NumberOfItems)
        {
        ret = this->Array[id];
	return 1;
        }
      return 0;
    }
      
  // Description:
  // Find an item in the vector. Return one if it was found, zero if it was
  // not found. The location of the item is returned in res.
  int Find(T a, unsigned long &res) 
    {
      int i;
      for (i = 0; i < this->NumberOfItems; ++i)
        {
        if (this->Array[i] == a)
          {
          res = i;
          return 1;
          }
        }
      return 0;
    }

  // Description:
  // Find an item in the vector using a comparison routine. 
  // Return one if it was found, zero if it was
  // not found. The location of the item is returned in res.
  int Find(T a, CompareFunction compare, unsigned long &res) 
    {
      int i;
      for (i = 0; i < this->NumberOfItems; ++i)
        {
        if ( compare(this->Array[i], a) )
          {
          res = i;
          return 1;
          }
        }
      return 0;
    }
  
  // Description:
  // Return the number of items currently held in this container. This
  // different from GetSize which is provided for some containers. GetSize
  // will return how many items the container can currently hold.
  virtual unsigned long GetNumberOfItems() { return this->NumberOfItems; }
  
  // Description:
  // Returns the number of items the container can currently hold.
  virtual unsigned long GetSize() { return this->Size; }

  // Description:
  // Removes all items from the container.
  virtual void RemoveAllItems()
    {
    if (this->Array)
      {
      delete [] this->Array;
      }
    this->Array = 0;
    this->NumberOfItems = 0;
    this->Size = 0;
    }
  

protected:
  vtkVector() {
    this->Array = 0; this->NumberOfItems = 0; this->Size = 0; }
  ~vtkVector() {
    if (this->Array)
      {
      delete [] this->Array;
      }
  }
  unsigned long NumberOfItems;
  unsigned long Size;
  T *Array;
};
