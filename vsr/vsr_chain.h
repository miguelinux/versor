/*
 *  Chain.h
 *  VERSOR
 *
 *  Created by xpc on 9/29/10.
 *  Copyright 2010 wolftype. All rights reserved.
 *
 
3D KINEMATIC CHAIN can be closed or open (robot arm)
 
 > if one end is fixed, it is a MECHANISM
 
 > if a MECHANISM transmits power it is a MACHINE
 
*/

#ifndef VSR_CHAIN_H_INCLUDED
#define VSR_CHAIN_H_INCLUDED

#include "vsr_frame.h"
#include "vsr_twist.h"
#include <map>
#include <string>

namespace vsr{

  using std::map;

/* namespace Kinematic { */
/*   enum Type{ */
/*       REVOLUTE, PRISMATIC, CYLINDRICAL, HELICAL, PLANAR, SPHERICAL */
/*   } */
/* } */

struct Joint : public Frame {
  
  enum Type{
      REVOLUTE, PRISMATIC, CYLINDRICAL, HELICAL, PLANAR, SPHERICAL
  };

// static map<string, Type> LowerPairMap;


  Type type;

  Joint( Type t = Joint::SPHERICAL, const Frame& f = Frame() ) : Frame(f), type(t) {}

  virtual Frame operator()() const = 0;

};

/* map<string,Joint::Type> Joint::LowerPairMap = { */ 
/*   make_pair("R",Joint::REVOLUTE), */ 
/*   make_pair("P", Joint::PRISMATIC), */ 
/*   make_pair("C", Joint::CYLINDRICAL), */
/*   make_pair("H", Joint::HELICAL), */
/*   make_pair("S", Joint::SPHERICAL) , */
/*   make_pair("X", Joint::PLANAR) }; */


struct Prismatic {
  Prismatic() : mFrame() {}
  Frame operator() (VT amt) { return mFrame.moveZ(amt); }//pos() = frame.pos().trs ( frame.z() * amt ) ; }
  Frame mFrame;
};

struct Revolute : public Joint {
  Revolute( const Frame& f = Frame() ) : Joint ( Joint::REVOLUTE, f) {}
  Frame operator() (VT amt) { mAmt = amt; return rotXY( amt ); }
  virtual Frame operator()() const { return rotXY( mAmt ); }
  VT mAmt;
};

struct Cylindrical {
  Cylindrical() : mFrame() {}
  Frame operator() (VT slide, VT rotate) { return mFrame.moveZ(slide).rotXY( rotate ); }
  Frame mFrame;
};

struct Helical {
  Helical( VT period = PI, VT pitch = 1.0 ) : mFrame(), mPeriod( period), mPitch(pitch) {}
  Frame operator() (VT amt) { return Frame( Gen::mot( Twist::Along( mFrame.dlz(), mPeriod, mPitch ) * amt ) * mFrame.mot() ); }
  Frame mFrame;
  VT mPeriod, mPitch;
};

struct Planar {
  Planar() : mFrame() {}
  Frame operator() (VT dx, VT dy, VT rotate) { return mFrame.move(dx,dy).rotXY( rotate ); }
  Frame mFrame;
};
 
struct  Spherical : public Joint {
  Spherical( const Frame& f = Frame() ) : Joint(Joint::SPHERICAL, f) {}
  VT theta, phi;
  Frame operator() (VT rx, VT ry) { theta = rx; phi = ry; return Frame( pos(), Gen::rot(theta,phi) ); }
  virtual Frame operator() () const { return Frame( pos(), Gen::rot(theta,phi) ); } 
};
//spherical (sph coords)
//planar


  class Chain : public Frame {
      
     vector<Joint*> mJoint;   
    //Joint * mJoint;      ///< In Socket Transformation (RDHC, etc) SET THIS directly using joint(i) 
                         ///  (all others follow after calling fk() method)  
                         //

     vector<Frame> mLink;           
    //Frame * mLink;      ///< Relative Link to NEXT joint
    
      vector<Frame> mFrame;
   // Frame * mFrame;      ///< Absolute frames of Joints = prevFrame  * prevLink *  joint

    int mNum;
    
    void _init(){
      for (int i = 0; i < mNum; ++i){
        Vec v(0,1.0,0);
        mLink[i].pos() = Ro::null(v);//trs( Gen::trs(v) );
        mFrame[i].scale(.5);
        
        //cout << mLink[i].pos() << endl;
      }
            
       fk();
    }
    
    public:
    
      Chain(const string& s) {//: mFrame(NULL), mJoint(NULL), mLink(NULL) {
        
        //mNum = s.length();
        alloc(s);
        _init();
      }

      Chain(int n = 3) : mNum(n) {//, mFrame(NULL), mJoint(NULL), mLink(NULL) {
                
          alloc(n);
          _init();    
                  
      }       
        
      
        ~Chain(){
            //if (mFrame) delete[] mFrame;
            //if (mLink) delete[] mLink;
            //if (mJoint) delete[] mJoint;
              mFrame.clear();
              mLink.clear();
              mJoint.clear(); 
        }

        void alloc(const string& s){
           mNum = s.length();
           if (mNum>0){

               mFrame.clear();
               mLink.clear();
               mJoint.clear();  

               mFrame = vector<Frame>(mNum);//new Frame[n];
               mLink  = vector<Frame>(mNum);///new Frame[n];


               for (int i = 0; i < mNum; ++i){
                  
                  if( strncmp( &s[i], "R", 1 ) == 0) mJoint.push_back( new Revolute() );
                  else if (strncmp( &s[i], "S", 1 ) == 0 ) mJoint.push_back( new Spherical() );

                      /* case "R":  mJoint.push_back( new Revolute() ); */
                      /* break; */
                      /* case "P":  //mJoint.push_back( new Revolute() ); */
                      /* break; */
                      /* case "C": */
                      /* break; */ 
                      /* case "H": */
                      /* break; */
                      /* case "S": mJoint.push_back( new Spherical() ); */
                      /* break; */
                      /* case "X": */
                      /* break; */
                  /* } */
                
                // mJoint.push_back( Joint::LowerPairMap[ s[i] ] 
               }

              _init();
           }

        }
        
        void alloc(int n){
            mNum = n; 
            if (mNum>0){
              //if (mFrame ) delete[] mFrame;
              //if (mLink ) delete[] mLink;
              //if (mJoint ) delete[] mJoint; 
              
              mFrame.clear();
              mLink.clear();
              mJoint.clear();  

              mFrame = vector<Frame>(n);//new Frame[n];
              mLink  = vector<Frame>(n);///new Frame[n];
              for (int i = 0; i < mNum; ++i){
                Spherical * s = new Spherical();
                mJoint.push_back(s);
              }
             // mJoint = vector<Joint*>(n, new Spherical());///new Frame[n];
              _init(); 
            }           
        }
            
        void frameSet(){
            mJoint[0] -> set( mPos, mRot ); fk();
        }
        
      /* GETTERS AND SETTERS */
      int num() const { return mNum; }
      Frame& link(int k) { return mLink[k]; }            ///< set k's Link To Next Joint
      Frame& joint(int k) { return *mJoint[k]; }          ///< set kth joint's In Socket Transformation
      Frame& frame(int k) { return mFrame[k]; }          ///< set Absolute Displacement Motor

      Frame link(int k) const { return mLink[k]; }        ///< Get k's Link To Next joint 
      Frame joint(int k) const { return *mJoint[k]; }        ///< Get kth Joint's In Socket Transformation
      Frame frame(int k) const { return mFrame[k]; }        ///< Get Absolute Displacement Motor

      
      Frame& operator [] (int k) { return mFrame[k]; }        ///< Set kth Absolute Frame
      Frame operator [] (int k) const { return mFrame[k]; }    ///< Get kth Absolute Frame
      
      /* SURROUNDS */
      /// Sphere Centered at Joint K Going Through Joint K+1 
      Dls nextDls(int k) const{
        return Ro::dls(mFrame[k].pos(), mLink[k].vec().norm() );//mFrame[k+1].pos());
      }
      /// Sphere Centered at Joint K Going Through Joint K-1
      Dls prevDls(int k) const{
        return Ro::dls(mFrame[k].pos(), mLink[k-1].vec().norm());//mFrame[k-1].pos());
      }
            /// Sphere at Point p through Joint K
            Dls goalDls(const Pnt& p, int k){
                return Ro::dls(p, mLink[k].vec().norm());
            }
      /// Sphere at point p through last link (default, or set arbitary link)
      Dls lastDls(const Pnt& p){
        return Ro::dls(p,mLink[mNum-1].vec().norm());
      }
        
            /* Possible Points */
            
            /// Pnt at position t along Link idx
            Pnt at(int idx, double t = 0.0){
                return Ro::null( Interp::linear<Vec>( mFrame[idx].vec(), mFrame[idx+1].vec(), t) );
            }
            
            Frame& base() { return mFrame[0]; }
            Frame& first() { return mFrame[0]; }        
            Frame& last() { return mFrame[mNum -1]; }
      
      /// Vert xy Plane Containing Root Target Point v ( NORMALIZED )
      Dlp xy(const Pnt& p) {
        return Op::dl(frame(0).pos()^Ro::null(0,1,0)^p^Inf(1)).unit();
      }
      /// Horiz xz Plane Containing Target Point v
      Dlp xz(const Pnt& p)  {
        return Dlp(0,1,0,p[1]);
      }

      /// Dual Line Forward: Line from kth frame to kth Link
      Dll linkf(int k) { return Op::dl( mFrame[k].pos() ^ mFrame[k+1].pos() ^ Inf(1) ).runit() ; }      
      /// Dual Line Forward: Line from kth frame to kth+1 joint
      Dll linf(int k) { return Op::dl( mFrame[k].pos() ^ mFrame[k+1].pos() ^ Inf(1) ).runit() ; }
      /// Dual Line Backward: Line from kth frame to kth-1 joint
      Dll linb(int k ) { return Op::dl( mFrame[k].pos() ^ mFrame[k-1].pos() ^ Inf(1) ).runit() ; }
      /// Dual Line From Kth Joint to Input Target (Default is From Last joint)
      Dll lin(const Pnt& p ) { return Op::dl( mFrame[mNum-1].pos() ^ p ^ Inf(1) ).runit() ; }

      /// relative (lagrangian) at kth joint
      Mot rel(int idx){
         if (idx==0) return mJoint[0] -> mot();

         return mLink[idx-1].mot() * mJoint[idx] -> mot();
      }
      
      /// Forward Kinematics: Absolute Concatenations of previous frame, previous link, and current joint
            void fk() {  
                Motor mot = mJoint[0] -> mot();
                mFrame[0].mot( mot );
                for (int i = 1; i < mNum; ++i){    
                  Mot rel =  mLink[i-1].mot() * mJoint[i] -> mot();//mLink[i-1].mot() * mJoint[i].mot();
                  mFrame[i].mot( mFrame[i-1].mot() * rel );// * mFrame[i-1].mot() );// mFrame[i-1].mot() * rel ) ;
                }
            }        
        
            /// Forward Kinematics: Selective From begin joint to end joint
      void fk(int begin, int end){
//                Mot mot = mJoint[0].mot();
        for (int i = begin; i < end; ++i){
          Mot m =  mFrame[i-1].mot()  * mLink[i-1].mot() * mJoint[i]->mot();
          mFrame[i].mot( m );        
        }
      }

            void ik(int end, int begin){
                
            }
        
            void ifabrik(const Pnt& p, int end, int begin, double err = .01){
                //squared distance between last frame and goal p
                Sca s = mFrame[end].pos() <= p * -2.0;
                
                //Temporary Goal
                Pnt goal = p;
                Pnt base = mFrame[begin].pos();
                
                int n = 0;
                
                //repeat until distance is decreased below threshold, or give up after 20 iterations
                while (s[0] > err){
                    
                    Pnt tmpGoal = goal;
                    Pnt tmpBase = base;
                    
                    //some objects
                    static Dls dls; //surround
                    static Dll dll; //line
                    static Par par; //intersection of line ^ surround
                    
                    //forward reaching
                    for (int i = end; i < begin; ++i){
                        mFrame[i].pos( tmpGoal );          //set position of ith frame
                        dls = nextDls(i);               //set boundary sphere through i-1 th frame;
                        dll = linf(i);                  //get line from tmp to i-1th frame
                        par = (dll ^ dls).dual();       //get point pair intersection of line and boundary sphere
                        tmpGoal = Ro::split(par,true);         //extract point from point pair intersection
                    }
                    
                    //backward correction
                    for (int i = begin; i > end; --i){
                        dls = prevDls(i);                   //set boundary sphere through i+1 th frame
                        dll = linb(i);                      //get line to i+1th frame;
                        par = (dll ^ dls).dual();           //get point pair intersection of line and boundary sphere
                        tmpBase = Ro::split(par,true);
                        mFrame[i-1].pos(tmpBase);             //set position of i+1th frame
                    }
                    
                    //squared distance between end frame and goal p
                    s = mFrame[ end ].pos() <= p * -2.0;
                    
                    n++;  if (n > 20) {  break; }
                }
                
                //calculate joint angles
                calcJoints();               
            }

            /// "FABRIK" Iterative Solver [see paper "Inverse Kinematic Solutions using Conformal Geometric Algebra", by Aristodou and Lasenby] feed target point, end frame and beginning frame,
            void fabrik(const Pnt& p, int end, int begin, double err = .01){
                
                //squared distance between last frame and goal p
                Sca s = mFrame[end].pos() <= p * -2.0;
                
                //Temporary Goal
                Pnt goal = p;
                //Base
                Pnt base = mFrame[begin].pos();
                
                int n = 0;
                
                //repeat until distance is decreased to within error threshold, or give up after 20 iterations
                while (s[0] > err){
                    
                    Pnt tmpGoal = goal;
                    Pnt tmpBase = base;
                    
                    static Dls dls; //surround
                    static Dll dll; //line
                    static Par par; //intersection of line ^ surround
                    
                    //backward reaching
                    for (int i = end; i > begin; --i){
                        mFrame[i].pos( tmpGoal );        //set position of ith frame
                        dls = prevDls(i);               //set boundary sphere through i-1 th frame;
                        dll = linb(i);                  //get line from tmp to i-1th frame
                        par = (dll ^ dls).dual();       //get point pair intersection of line and boundary sphere
                        tmpGoal = Ro::split(par,true);  //extract point from point pair intersection
                    }
                    
                    //forward correction
                    for (int i = begin; i < end; ++i){
                        dls = nextDls(i);                   //set boundary sphere through i+1 th frame
                        dll = linf(i);                      //get line to i+1th frame;
                        par = (dll ^ dls).dual();           //get point pair intersection of line and boundary sphere
                        tmpBase = Ro::split(par,true);
                        mFrame[i+1].pos(tmpBase);           //set position of i+1th frame
                    }
                    
                    s = mFrame[ end ].pos() <= p * -2.0;
                     
                    n++;  if (n > 20) {  break; }
                }

                //calculate joint angles
                calcJoints();

            }
        

        
            /// Derive Joint Rotations from Current Positions
            void calcJoints(int start = 0){


                Vec t = Vec::y;
                Rot R(1,0,0,0);
                
                //Where we are in current rotation scheme
                for (int i = 0; i < start; ++i){
                    //cout << "update" << endl; 
                    t = t.sp( mFrame[i].rot() );
                }

                //From Here forward, what we need to get where we want to go
                for (int i = start; i < mNum-1; ++i){
                    //DRV of LINK
                    Vec b = Op::dle( Biv( linf(i) ) );                    
                    Rot nr = Gen::ratio( t, b ); //What it takes to turn the current integration there
                    R = nr * R;
                    mFrame[i].rot( R );
                    t = t.sp( nr ); //angle is integrated
                }
                
                //Set Base Joint
                mJoint[0] -> rot( mFrame[0].rot() );
                
                for (int i = 1; i < mNum; ++i){                    
                    //reverse engineer
                    Rot Rt = (!mFrame[i-1].rot()) * mFrame[i].rot();
                    mJoint[i] -> rot( Rt ); 
                }

            }
        
            ///Satisfy Specific Angle Constraint at frame k
            void angle(int k, double theta){
                
                Rot R =  mJoint[k] -> rot();
                //double t = Gen::iphi ( R );
                                
                Biv b = Biv( R ) * -1;// * ( (t > 1) ? ); // note: check Op:lg and Gen::log_rot (maybe mult by -1 there as well)

//                cout << b << endl; 
//                cout << Op::lg( R ) << endl; 
//                cout << Op :: pl( R ) << endl; 
                
                Rot nr = Gen::rot( b.unit() * theta );
                
                mJoint[k] -> rot( nr );
                
                //forward kinematics
                fk(k,mNum);
                
            }
    
      
      /// Derive New Relative Link Frames from current Positions
      void calcLinks(){
        for (int i = 0; i < mNum-1; ++i){
          mLink[i].pos() = Ro::null( mFrame[i+1].vec() - mFrame[i].vec() ); 
                    //mLink[i].mot( mFrame[i+1].mot() / mFrame[i].mot() ); 
        }
      } 
        
  
  };

} //vsr::

#endif
