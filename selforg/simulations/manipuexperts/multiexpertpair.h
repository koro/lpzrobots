/***************************************************************************
 *   Copyright (C) 2005 by Robot Group Leipzig                             *
 *    martius@informatik.uni-leipzig.de                                    *
 *    fhesse@informatik.uni-leipzig.de                                     *
 *    der@informatik.uni-leipzig.de                                        *
 *                                                                         *
 *   ANY COMMERCIAL USE FORBIDDEN!                                         *
 *   LICENSE:                                                              *
 *   This work is licensed under the Creative Commons                      *
 *   Attribution-NonCommercial-ShareAlike 2.5 License. To view a copy of   *
 *   this license, visit http://creativecommons.org/licenses/by-nc-sa/2.5/ *
 *   or send a letter to Creative Commons, 543 Howard Street, 5th Floor,   *
 *   San Francisco, California, 94105, USA.                                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 *   This version of multiexpert has the following features                *
 *    n experts, competition pairwise between winner and companion         *
 *    prediction error of companion is increased for competition (hysteris)*
 *    expert get mature, when winning and low error,                       *
 *     i.e. learning rate decreases                                        *
 *    always the agent with lowest maturation is selected  as new companion*
 *    only winner and companion are allowed to learn                        *
 *                                                                         *
 *                                                                         *
 *   $Log$
 *   Revision 1.1  2008-04-24 08:42:34  martius
 *   multiexpert with manipulandum data
 *
 *   Taken from multisat 1.11
 *
 ***************************************************************************/
#ifndef __MULTIEXPERTPAIR_H
#define __MULTIEXPERTPAIR_H


#include <assert.h>
#include <math.h>

#include <selforg/matrix.h>
#include <selforg/noisegenerator.h>
#include <selforg/multilayerffnn.h>

typedef struct MultiExpertPairConf {
  int numHidden;        ///< number of hidden units in the satelite networks
  double eps0;          ///< learning rate for satelite networks
  double tauE1;         ///< time horizont for short averaging error
  double tauE2;         ///< time horizont for long averaging error
  double lambda_w;      ///< discount for winner prediction error (hysteresis) in procent
  double tauI;          ///< time horizont for imaturation of all experts
  double tauW;          ///< time horizont for winner learning rate decay (maturation)
  int    numSats;       ///< number of satelite networks
} MultiExpertPairConf;

/// Satelite network struct
typedef struct Sat {
  Sat(MultiLayerFFNN* _net, double _eps);
  MultiLayerFFNN* net;
  double eps;  
  double lifetime;
} Sat;

/**
 * class for robot controller 
 * using several feedforward networks (satelite) and one selforg controller
 */
class MultiExpertPair : public  AbstractModel{

public:
  MultiExpertPair(const MultiExpertPairConf& conf = getDefaultConf());

  virtual ~MultiExpertPair();

  virtual void init(unsigned int inputDim, unsigned  int outputDim, 
		    double unit_map = 0.0, RandGen* randGen = 0);

  virtual unsigned int getInputDim() const { return inputDim;}
  virtual unsigned int getOutputDim() const  { return outputDim;}

  virtual void damp(double damping) {};

  virtual const matrix::Matrix process (const matrix::Matrix& input);

  virtual const matrix::Matrix learn (const matrix::Matrix& input, 
				      const matrix::Matrix& nom_output, 
				      double learnRateFactor = 1);

  // !!!!!!!!!!!!!!!!!!! MISC STUFF !!!!!!!!
  
  /// stores the sat networks into seperate files
  void storeSats(const char* filestem); 

  /************** CONFIGURABLE ********************************/
  virtual paramval getParam(const paramkey& key) const;
  virtual bool setParam(const paramkey& key, paramval val);
  virtual paramlist getParamList() const;


  /**** STOREABLE ****/
  /** stores the controller values to a given file. */
  virtual bool store(FILE* f) const;
  /** loads the controller values from a given file. */
  virtual bool restore(FILE* f);  

  /**** INSPECTABLE ****/
  virtual std::list<iparamkey> getInternalParamNames() const;
  virtual std::list<iparamval> getInternalParams() const;  
  virtual std::list<ILayer> getStructuralLayers() const;
  virtual std::list<IConnection> getStructuralConnections() const;

  static MultiExpertPairConf getDefaultConf(){
    MultiExpertPairConf c;
    c.numHidden = 2;
    c.eps0=0.005;
    //    c.lambda_comp = 0;
    // c.tauC = 10.0/c.eps0; 
    c.tauE1 = 20; 
    c.tauE2 = 200; 
    c.tauW  = 1000; 
    c.tauI  = 50000; 
    c.numSats=20;
    // c.penalty=5;
    return c;
  }


protected:  
  int inputDim;
  int outputDim;

  std::vector <Sat> sats; ///< satelite networks
  int winner;    ///< index of winner network
  int companion; ///< index of companion network
  matrix::Matrix nomSatOutput; ///< norminal output of satelite networks (x_t,y_t)^T
  matrix::Matrix satInput;     ///< input to satelite networks (x_{t-1}, xp_{t-1}, y_{t-1})^T

  bool runcompetefirsttime;       ///< flag to initialise averaging with proper value
  matrix::Matrix satErrors;       ///< actual errors of the sats
  matrix::Matrix satAvg1Errors;   ///< short averaged errors of sats
  matrix::Matrix satAvg2Errors;   ///< long averaged errors of sats
  matrix::Matrix satModErrors;    ///< modulated (avg1) errors of sats
  matrix::Matrix satEpsMod;       ///< modulated eps of sats
  
  MultiExpertPairConf conf;
  bool initialised;
  bool managementInterval;
  int t;
protected:
  /// satelite networks competition, return vector of prediction errors of sat networks
  matrix::Matrix compete(const matrix::Matrix& input, 
			 const matrix::Matrix& nom_output);

  void management();

  static double min(void* m, double d);

};

#endif