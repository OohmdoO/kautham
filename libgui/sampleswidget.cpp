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
     
#include "sampleswidget.h"
#include <libsampling/sampling.h>
#include <libsutil/lcprng.h>
#include <sstream>


namespace Kautham{


	SamplesWidget::SamplesWidget(SampleSet* samples, Sampler* sampler, Problem* prob){
    _samples = samples;
    _sampler = sampler;
    _ptProblem = prob;
    gridLayout = new QGridLayout(this);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    vboxLayout = new QVBoxLayout();
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    label = new QLabel(this);
    label->setObjectName(QString::fromUtf8("label"));

    vboxLayout1->addWidget(label);

    cboSampleList = new QComboBox(this);
    cboSampleList->setObjectName(QString::fromUtf8("cboSampleList"));
    cboSampleList->setEditable(false);

    vboxLayout1->addWidget(cboSampleList);


    vboxLayout->addLayout(vboxLayout1);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    btnCollision = new QPushButton(this);
    btnCollision->setObjectName(QString::fromUtf8("btnCollision"));
    hboxLayout->addWidget(btnCollision);

    btnDistance = new QPushButton(this);
    btnDistance->setObjectName(QString::fromUtf8("btnDistance"));
    hboxLayout->addWidget(btnDistance);

    vboxLayout->addLayout(hboxLayout);

    btnAddCurrent = new QPushButton(this);
    btnAddCurrent->setObjectName(QString::fromUtf8("btnAddCurrent"));
    btnAddCurrent->setText("Add current configuration as a sample");
    vboxLayout->addWidget(btnAddCurrent);

    btnRemoveCurrent = new QPushButton(this);
    btnRemoveCurrent->setObjectName(QString::fromUtf8("btnRemoveCurrent"));
    btnRemoveCurrent->setText("Remove current sample from collection");
    vboxLayout->addWidget(btnRemoveCurrent);

    QPushButton *btnRemoveAll = new QPushButton(this);
    btnRemoveAll->setObjectName(QString::fromUtf8("btnRemoveAll"));
    btnRemoveAll->setText("Remove all samples in collection");
    vboxLayout->addWidget(btnRemoveAll);

    QPushButton *btnRemoveAllEx2 = new QPushButton(this);
    btnRemoveAllEx2->setObjectName(QString::fromUtf8("btnRemoveAllEx2"));
    btnRemoveAllEx2->setText("Copy the two first in a new collection");
    vboxLayout->addWidget(btnRemoveAllEx2);

    QPushButton *btnUpdateList = new QPushButton(this);
    btnUpdateList->setObjectName(QString::fromUtf8("btnUpdateList"));
    btnUpdateList->setText("Update the samples list of collection");
    vboxLayout->addWidget(btnUpdateList);

    groupBox = new QGroupBox(this);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    gridLayout1 = new QGridLayout(groupBox);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    vboxLayout2 = new QVBoxLayout();
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    rbtnAdd = new QRadioButton(groupBox);
    rbtnAdd->setObjectName(QString::fromUtf8("rbtnAdd"));
    rbtnAdd->setChecked(true);
    vboxLayout2->addWidget(rbtnAdd);

    rbtnNew = new QRadioButton(groupBox);
    rbtnNew->setObjectName(QString::fromUtf8("rbtnNew"));

    vboxLayout2->addWidget(rbtnNew);

    gridLayout1->addLayout(vboxLayout2, 0, 0, 1, 1);

    vboxLayout->addWidget(groupBox);

    groupBox_2 = new QGroupBox(this);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    gridLayout2 = new QGridLayout(groupBox_2);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    vboxLayout3 = new QVBoxLayout();
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));

	rbtnRandom = new QRadioButton(groupBox_2);
    rbtnRandom->setObjectName(QString::fromUtf8("rbtnRandom"));
    rbtnRandom->setChecked(true);
    vboxLayout3->addWidget(rbtnRandom);

    rbtnSDK = new QRadioButton(groupBox_2);
    rbtnSDK->setObjectName(QString::fromUtf8("rbtnSDK"));
    vboxLayout3->addWidget(rbtnSDK);

    rbtnHalton = new QRadioButton(groupBox_2);
    rbtnHalton->setObjectName(QString::fromUtf8("rbtnHalton"));
    vboxLayout3->addWidget(rbtnHalton);
	
	rbtnGaussian = new QRadioButton(groupBox_2);
	rbtnGaussian->setObjectName(QString::fromUtf8("rbtnGaussian"));
	vboxLayout3->addWidget(rbtnGaussian);

	rbtnGaussianLike = new QRadioButton(groupBox_2);
	rbtnGaussianLike->setObjectName(QString::fromUtf8("rbtnGaussianLike"));
	vboxLayout3->addWidget(rbtnGaussianLike);

    gridLayout2->addLayout(vboxLayout3, 0, 0, 1, 1);

    vboxLayout->addWidget(groupBox_2);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    label_3 = new QLabel(this);
    label_3->setObjectName(QString::fromUtf8("label_3"));

    hboxLayout1->addWidget(label_3);

    txtAmount = new QLineEdit(this);
    txtAmount->setObjectName(QString::fromUtf8("txtAmount"));

    hboxLayout1->addWidget(txtAmount);


    vboxLayout->addLayout(hboxLayout1);

    btnSampling = new QPushButton(this);
    btnSampling->setObjectName(QString::fromUtf8("btnSampling"));

    vboxLayout->addWidget(btnSampling);


    gridLayout->addLayout(vboxLayout, 0, 0, 1, 1);

    label->setText("List of current samples");
    btnCollision->setText("Test Collision");
    btnDistance->setText("Test Distance");
    groupBox->setTitle("Strategy");
    rbtnAdd->setText("Add to collection");
    rbtnNew->setText("New collection");
    groupBox_2->setTitle("Engine");
    rbtnRandom->setText("Random");
    rbtnSDK->setText("SDK");
    rbtnHalton->setText("Halton");
	rbtnGaussian->setText("Gaussian");
	rbtnGaussianLike->setText("Gaussian-Like");
    label_3->setText("Samples amount");
    btnSampling->setText("Get Samples");

    connect(btnCollision, SIGNAL( clicked() ), this, SLOT( collisionCheck() ) ); 
    connect(btnDistance, SIGNAL( clicked() ), this, SLOT( distanceCheck() ) );
    connect(btnSampling, SIGNAL( clicked() ), this, SLOT( sampling() ) );
    connect(btnAddCurrent, SIGNAL( clicked() ), this, SLOT( addCurrent() ) );
    connect(btnRemoveCurrent, SIGNAL( clicked() ), this, SLOT( removeCurrent() ) );
    connect(btnRemoveAll, SIGNAL( clicked() ), this, SLOT( removeAll() ) );
    connect(btnRemoveAllEx2, SIGNAL( clicked() ), this, SLOT( removeAllEx2() ) );
    connect(btnUpdateList, SIGNAL( clicked() ), this, SLOT( updateSampleList() ) );
    connect(cboSampleList, SIGNAL( currentIndexChanged( int )), this, SLOT( showSample( int )));

	
    connect(rbtnSDK, SIGNAL( clicked() ), this, SLOT( changeEngine() ) );
    connect(rbtnHalton, SIGNAL( clicked() ), this, SLOT( changeEngine() ) );
    connect(rbtnRandom, SIGNAL( clicked() ), this, SLOT( changeEngine() ) );
    connect(rbtnGaussian, SIGNAL( clicked() ), this, SLOT( changeEngine() ) );
    connect(rbtnGaussianLike, SIGNAL( clicked() ), this, SLOT( changeEngine() ) );

    updateSampleList();
    
	}
	
	void SamplesWidget::changeEngine()
	{
		if( rbtnRandom->isChecked() && typeid(*_sampler) != typeid(RandomSampler) ){
			//if( _sampler != NULL )
				delete _sampler;
            _sampler = new RandomSampler(_ptProblem->wSpace()->getNumRobControls());
            _ptProblem->setSampler(_sampler);
		}
      
		
		if( rbtnSDK->isChecked() && typeid(*_sampler) != typeid(SDKSampler) ){
			//if( _sampler != NULL )
				delete _sampler;
            _sampler = new SDKSampler(_ptProblem->wSpace()->getNumRobControls(),2);
            _ptProblem->setSampler(_sampler);
		}
		
		if( rbtnHalton->isChecked() && typeid(*_sampler) != typeid(HaltonSampler) ){
			//if( _sampler != NULL )
				delete _sampler;
            _sampler = new HaltonSampler(_ptProblem->wSpace()->getNumRobControls());
            _ptProblem->setSampler(_sampler);
		}

		
		if( rbtnGaussian->isChecked() && typeid(*_sampler) != typeid(GaussianSampler) ){
			//if( _sampler != NULL )
				delete _sampler;
            //_sampler = new GaussianSampler(_ptProblem->wSpace()->getNumRobControls());
            _sampler = new GaussianSampler(_ptProblem->wSpace()->getNumRobControls(),0.1,_ptProblem->wSpace());
            _ptProblem->setSampler(_sampler);
		}

		
		if( rbtnGaussianLike->isChecked() && typeid(*_sampler) != typeid(GaussianLikeSampler) ){
			//if( _sampler != NULL )
				delete _sampler;
            _sampler = new GaussianLikeSampler(_ptProblem->wSpace()->getNumRobControls(),2, _ptProblem->wSpace());
            _ptProblem->setSampler(_sampler);
		}
  }


  void SamplesWidget::collisionCheck(){
    string str;
    stringstream sstr;
    if(cboSampleList->count() >= 1){
      Sample* tmpSam = _samples->getSampleAt((cboSampleList->currentText()).toInt());
      if( _ptProblem->wSpace()->collisionCheck(tmpSam))
        str = "COLLISION";
      else
        str = "FREE";
      sstr << "The sample No: " << cboSampleList->currentText().toUtf8().constData() 
        << " is: " << str.c_str();
      writeGUI(sstr.str());
    }else
      writeGUI("First create a sample");

  }

  void SamplesWidget::distanceCheck(){
    stringstream sstr;
    string p;
    if(cboSampleList->count() >= 1){
      sstr.precision(10);
      Sample* tmpSam = _samples->getSampleAt((cboSampleList->currentText()).toInt());
      vector<KthReal>* val = _ptProblem->wSpace()->distanceCheck(tmpSam);

      for(unsigned int i = 0; i<val->size(); i++)
        sstr << val->at(i) << ", " ;
      p = sstr.str();
	    p = p.substr(0,p.length()-2);

      sstr.clear();
      sstr << "For sample No: " << cboSampleList->currentText().toUtf8().constData() 
        << " the distance check is: " << p.c_str();

      writeGUI(sstr.str());
    }else
      writeGUI("First create a sample");
  }

  void SamplesWidget::addCurrent(){
    char sampleDIM = _ptProblem->wSpace()->getNumRobControls();
    Sample* tmpSam = new Sample(sampleDIM);
    tmpSam->setCoords(_ptProblem->getCurrentRobControls());
    if( ! _ptProblem->wSpace()->collisionCheck(tmpSam)){
		  _samples->add(tmpSam);
		  updateSampleList();
	  }else{
      writeGUI("Samples not added - COLLISION configuration!");
	    return;
	  }
  }

  void SamplesWidget::removeCurrent(){
    if(!_samples->removeSampleAt((cboSampleList->currentText()).toInt()))
      writeGUI("an error has been made. Please try again");
    updateSampleList();
  }

  void SamplesWidget::removeAll(){
    _samples->clear();
	if(typeid(*_sampler) == typeid(GaussianLikeSampler)) ((GaussianLikeSampler*)_sampler)->clear();
    updateSampleList();
  }

  void SamplesWidget::removeAllEx2(){
    if( _samples->getSize() > 0){
      char sampleDIM = _ptProblem->wSpace()->getNumRobControls();
      Sample *tmpSamInit = new Sample(sampleDIM); 
      Sample *tmpSamGoal = new Sample(sampleDIM);
      tmpSamInit->setCoords(_samples->getSampleAt(0)->getCoords());
      tmpSamGoal->setCoords(_samples->getSampleAt(1)->getCoords());
      _samples->clear();
	  if(typeid(*_sampler) == typeid(GaussianLikeSampler)) 
	  {
		  int i=9;
		  ((GaussianLikeSampler*)_sampler)->clear();
	  }
      _samples->add(tmpSamInit);
      _samples->add(tmpSamGoal);
	  if(_samples->isAnnSet()) _samples->loadAnnData();
    }
	else
	{
      _samples->clear(); 
	  if(typeid(*_sampler) == typeid(GaussianLikeSampler)) ((GaussianLikeSampler*)_sampler)->clear();
	}

    updateSampleList();
  }

  void SamplesWidget::sampling(){
    Sample *tmpSam;
    QString qstr;
    int numFree = 0;
    int samNum = (txtAmount->text()).toInt();
    
    if( rbtnNew->isChecked() && _samples->getSize() > 0)
      removeAllEx2();

    
	//
	//

    for(int i = 0; i < samNum; i++){
      tmpSam = _sampler->nextSample();
      tmpSam->setFree(!_ptProblem->wSpace()->collisionCheck(tmpSam));
      if(tmpSam->isFree()){
        numFree++;
        _samples->add(tmpSam);
      }
	  else
        delete tmpSam;
    }

    qstr = txtAmount->text() + " samples generated, " 
      + QString().setNum(numFree) + " samples free.";
    writeGUI( qstr.toUtf8().constData());
    txtAmount->setText("");
    updateSampleList();
  }

  void SamplesWidget::updateSampleList(){
    if(cboSampleList->count() < _samples->getSize()){
      for(int i = cboSampleList->count(); i < _samples->getSize(); i++){
        cboSampleList->addItem( QString().setNum(i) );
      }
    }else{
      for(int i = cboSampleList->count(); i > _samples->getSize(); i--){
        cboSampleList->removeItem( i - 1 );
      }
    }
  }

  void SamplesWidget::writeGUI(string text){
    emit sendText(text);
  }

  void SamplesWidget::showSample(int index){
    if(index >= 0 && index < _samples->getSize() )
      _ptProblem->wSpace()->moveRobotsTo(_samples->getSampleAt(index));
  }

}

