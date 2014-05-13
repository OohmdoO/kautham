/*************************************************************************\
   Copyright 2014 Institute of Industrial and Control Engineering (IOC)
                 Universitat Politecnica de Catalunya
                 BarcelonaTech
    All Rights Reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the
    Free Software Foundation, Inc.,
    59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 \*************************************************************************/

/* Author: Alexander Perez, Jan Rosell */



#if !defined(_SEQUENCE_H)
#define _SEQUENCE_H

#include "wmat.h"
#include "tmat.h"

namespace Kautham {


/** \addtogroup Sampling
 *  @{
 */

  //! This class provides the simply and fast way to use the deterministic sequence.
  //! This class is the minimal implementation of the deterministic sequence algorithm.
  //! More information about it, will be found in:
  //! Jan Rosell, Maximo Roa, Alexander Perez, and Fernando Garcia. A general deterministic sequence for sampling 
  //! d-dimensional configuration spaces. Journal of Intelligent and Robotic Systems, 50(4):361 - 373, December 2007.
  //! (omitted accents)

  class Sequence{
  public:
    //! Simply constructor. 
    //! This constructor is a simply way to obtain the SDK generator.  
    //! Be careful with dim and M because (int)2^(dim*M) should be represented correctly.
    //! If randoOffset is true, the first code of sequence is random, otherwise it is zero.
    
    Sequence(int dim, int M, bool randOffset=false);//true);

    //! Simply destructor.
    ~Sequence(void);

    //! This method returns the code of the cell with grid coordinates "indexes".
    unsigned long getCode(char *indexes);

    //! This method returns an array with the indexes of a cell with code "code".
    char* getIndexes(unsigned long int code);

    //! Returns the indexes corresponding to the last generated cell code of the sequence.
    char* getIndexes(void);

    //! Returns the new code in the sequence.
    unsigned long getSequenceCode();

    //! Returns the code corresponding to the \f$K^{th}\f$ sample of the sequence.
    unsigned long getSequenceCode(unsigned long K);

    //! Returns the V matrix for a cell with code "code". V is the matrix of index in binary representation.
    char** getVMatrix(unsigned long int code);

    //! Returns the V matrix corresponding  to the last sample generated of the sequence.
    char** getVMatrix(void);

    //! This method sets the W matrix.
    inline void setW(WMat &w){_wMat = &w;};

    //! This method returns a pointer to the W matrix.
    inline WMat* getW(){return _wMat;};

    //! This method sets the T matrix.
    inline void setT(TMat &t){_tMat = &t;};

    //! This method returns a pointer to the T matrix.
    inline TMat* getT(){return _tMat;};
  private:
    //! This is the index of the sequence.
    unsigned long int _index;

    //! This is the last code generated by the sequence.
    unsigned long int _lastCode;

    //! This is the initial random offset for the sequence.
    unsigned long int _offset;

    //! This is the maximum number of cells. It is \f$2^{Dim*M}\f$.
    unsigned long _maxNumCells;

    //! This is the M value. This is the maximum sampling level.
    char _maxSamplingLevel;

    //! This is the dimension of the space to be sampled.
    char _dim;

    //! Pointer to T matrix.
    TMat* _tMat;

    //! Pointer to W matrix.
    WMat* _wMat;

    //! This is the pointer to the unique V matrix used to calculate the binary values for indexes of a cell. 
    char** _V;

    //! Pointer to the unique indexes matrix that contains the indexes of a cell.
    char* _indexes;
  };

  /** @}   end of Doxygen module "Sampling" */
}
#endif //_SEQUENCE_H
