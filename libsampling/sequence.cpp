/***************************************************************************
*               Generated by StarUML(tm) C++ Add-In                        *
***************************************************************************/
/***************************************************************************
*                                                                          *
*           Institute of Industrial and Control Engineering                *
*                 Technical University of Catalunya                        *
*                        Barcelona, Spain                                  *
*                                                                          *
*                Project Name:       Kautham Planner                       *
*                                                                          *
*     Copyright (C) 2007 - 2009 by Alexander Pérez and Jan Rosell          *
*            alexander.perez@upc.edu and jan.rosell@upc.edu                *
*                                                                          *
*             This is a motion planning tool to be used into               *
*             academic environment and it's provided without               *
*                     any warranty by the authors.                         *
*                                                                          *
*          Alexander Pérez is also with the Escuela Colombiana             *
*          de Ingeniería "Julio Garavito" placed in Bogotá D.C.            *
*             Colombia.  alexander.perez@escuelaing.edu.co                 *
*                                                                          *
***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
 
 
#include "sequence.h"
#include <libsutil/lcprng.h>

namespace SDK {
  Sequence::Sequence(int dim, int M, bool randOffset){
    LCPRNG* gen1 = new LCPRNG(3141592621, 1, 0, ((unsigned int)time(NULL) & 0xfffffffe) + 1);//LCPRNG(15485341);//15485341 is a big prime number
    _index=0;
    _dim=dim;
    _maxSamplingLevel=M;
    _maxNumCells=0x01<<(_dim*_maxSamplingLevel);
    _V=NULL;
    _indexes = NULL;
    _lastCode=0;
    _offset=0;
    if(randOffset)_offset= (unsigned long)(gen1->d_rand()*_maxNumCells);
  }

  Sequence::~Sequence(void){
    if(_V != NULL){
			  for(int i=0;i<_dim;i++)
				  delete[] _V[i];
			  delete[] _V;
		  }
  }

  char* Sequence::getIndexes(void){
		return _indexes;
	}

  char* Sequence::getIndexes(unsigned long int code){
    if(code != _lastCode) 
      _V = getVMatrix(code);
		if(_indexes == NULL) _indexes = new char[_dim];
		for(int i=0; i<_dim; i++) {
			_indexes[i]=0;
			for(int j=0; j < _maxSamplingLevel; j++) {
				_indexes[i]+= _V[i][j]<<(_maxSamplingLevel-j-1);
			}
		}
		return _indexes;
	}

  unsigned long Sequence::getSequenceCode(unsigned long index){
		char **v = getVMatrix(index);
		char **tv = _tMat->multiply( v, _maxSamplingLevel );
		int loop = _index / _maxNumCells;
		unsigned long ret=0;
    for(int i=0; i<_dim; i++){
			for(int j=0; j<_maxSamplingLevel; j++){
				if(_wMat->getRow(i, j)!=-1){
					ret += _wMat->getRow(i, j ) * tv[i][_maxSamplingLevel-j-1];
				}
			}
    }
    if( _index <= _maxNumCells )
      return ret;
    else
      return ret+loop*_maxNumCells;
	}

  unsigned long Sequence::getSequenceCode(){
    return getSequenceCode( _offset + _index++ );
  }

  char** Sequence::getVMatrix(void){
    return _V;
  }

  char** Sequence::getVMatrix(unsigned long int code){
		long temp;
		code = code % _maxNumCells;
		if(_V == NULL){
			_V = new char*[_dim];
			for(int i=0;i<_dim;i++)
				_V[i]= new char[_maxSamplingLevel];
		}

		for(int i=0;i<_dim;i++)
			for( int j=0; j<_maxSamplingLevel; j++){
				temp = code & _wMat->getRow(i, j);
				if(temp == 0)
					_V[i][j] = 0;
				else
					_V[i][j] = 1;
			}
		return _V;
	}

  unsigned long Sequence::getCode(char *indexes){
    long temp;
    if(_V == NULL){
			_V = new char*[_dim];
			for(int i=0;i<_dim;i++)
				_V[i]= new char[_maxSamplingLevel];
		}
    for(char i=0; i< _dim; i++){
      for(char j=0; j<_maxSamplingLevel; j++){
        temp = ((long)indexes[i]) & (0x01<<(_maxSamplingLevel-j-1));
				if(temp == 0)
					_V[i][j] = 0;
				else
					_V[i][j] = 1;
      }
    }
    
		unsigned long ret=0;
    for(int i=0; i<_dim; i++){
			for(int j=0; j<_maxSamplingLevel; j++){
				if(_wMat->getRow(i, j)!=-1){
					ret += _wMat->getRow(i, j ) * _V[i][j];
				}
			}
    }
		return ret;
  }
}
