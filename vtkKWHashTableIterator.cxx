/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkKWHashTableIterator.cxx,v $
  Language:  C++
  Date:      $Date: 2002-02-04 20:33:55 $
  Version:   $Revision: 1.3 $


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

#include <stdlib.h>

#include "vtkKWHashTableIterator.h"
#include "vtkKWHashTable.h"
#include "vtkObjectFactory.h"


//------------------------------------------------------------------------------
vtkStandardNewMacro( vtkKWHashTableIterator );

vtkKWHashTableIterator::vtkKWHashTableIterator()
{
  this->HashTable        = 0;
  this->Bucket           = 0;
  this->HashTableElement = 0;
}

vtkKWHashTableIterator::~vtkKWHashTableIterator()
{
  this->SetHashTable(0);
}

unsigned long vtkKWHashTableIterator::GetKey()
{
  if ( this->HashTable && this->HashTableElement )
    {
    return this->HashTableElement->Key;
    }
  return 0;
}

void *vtkKWHashTableIterator::GetPointerKey()
{
  return reinterpret_cast<void *>( this->GetKey() );
}

void *vtkKWHashTableIterator::GetData()
{
  if ( this->HashTable && this->HashTableElement )
    {
    return this->HashTableElement->Data;
    }
  return 0;
}

int vtkKWHashTableIterator::Next()
{
  if ( !this->HashTable )
    {
    return 0;
    }
  if ( this->HashTableElement && this->HashTableElement->Next )
    {
    this->HashTableElement = this->HashTableElement->Next;
    this->Modified();
    return 1;
    }
  this->HashTableElement = 0;
  while ( !this->HashTableElement )
    {
    this->Bucket++;
    if ( this->HashTableElement )
      {
      break;
      }
    if ( this->Bucket >= this->HashTable->NumberOfBuckets )
      {
      return 0;
      }
    this->HashTableElement = this->HashTable->Table[this->Bucket];
    }
  this->Modified();
  return 1;
}

void vtkKWHashTableIterator::Reset()
{
  this->HashTableElement = 0;
  this->Bucket = 0;
  this->HashTableElement = this->HashTable->Table[this->Bucket];
  if ( ! this->Valid() )
    {
    this->Next();
    }
  this->Modified();
}

void vtkKWHashTableIterator::SetHashTable(vtkKWHashTable *table)
{
  if ( !table )
    {
    if ( this->HashTable )
      {
      this->HashTable->Delete();
      this->HashTable=0;
      }
    return;
    }
  if ( this->HashTable == table )
    {
    return;
    }
  if ( this->HashTable )
    {
    this->HashTable->UnRegister(this);
    }
  this->HashTable = table;
  this->HashTable->Register(this);
  this->Reset();
}
