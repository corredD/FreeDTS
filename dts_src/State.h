#if !defined(AFX_State_H_INCLUDED_)
#define AFX_State_H_INCLUDED_
#include "inclusion.h"
#include "triangle.h"
#include "vertex.h"
#include "links.h"
#include "SimDef.h"
#include "CreateMashBluePrint.h"
#include "Inclusion_Interaction_Map.h"
#include "CouplingtoFixedGlobalCurvature.h"
#include "SpringPotentialBetweenTwoGroups.h"
#include "PositionRescaleFrameTensionCoupling.h"
#include "CmdVolumeCouplingSecondOrder.h"
#include "CoupleToWallPotential.h"
#include "LinkFlipMC.h"
#include "VertexMCMove.h"
#include "InclusionMCMove.h"
#include "Restart.h"
#include "Nfunction.h"
#include "ActiveTwoStateInclusion.h"
#include "Apply_Osmotic_Pressure.h"
#include "Apply_Constant_Area.h"

/*
 Weria Pezeshkian (weria.pezeshkian@gmail.com)
 Copyright (c) Weria Pezeshkian
 This class reads the inputs and distrubutes the tasks based on inputs provided and makes all the initials variables
 It is called by Job class: 
 */
struct STRUC_TRJTSI {    // data structure for tsi trajectory file
    int tsiPeriod;
    int tsiPrecision;
    std::string tsiFolder_name;
    bool tsiState;
};
struct STRUC_RESTART {  // restart, how often write a file and check if this is a restart sim
    int  restartPeriod;
    bool restartState;
    std::string restartFilename;

};
struct STRUC_TRJBTS { //data structure for bts trajectory file (binary file format)
    int btsPeriod;
    int btsPrecision;
    std::string btsFile_name;
    bool btsState;
};
struct STRUC_VOLUME {  // data structure for inputs apply pressure and volume constrint to the system
    bool State;
    int EQSteps;
    double DeltaP;
    double K;
    double targetV;
};
struct STRUC_OSMOTIC {  // data structure for inputs of osmotic pressure algorithm
    bool State;
    std::string Type;
    int EQSteps;
    double Gamma;   // TargetV = GammaV_0;  V_0 = 1/6*(A0^1.5)/pi^0.5 ; A_0 = N_T*3sqrt(3)/4
    double P0;

};
struct STRUC_ConstantArea {  // data structure for inputs of osmotic pressure algorithm
    bool State;
    int EQSteps;
    double Gamma;   // TargetV = GammaV_0;  V_0 = 1/6*(A0^1.5)/pi^0.5 ; A_0 = N_T*3sqrt(3)/4
    double K0;
    
};
struct STRUC_FRAMETENSION { // data structure for inputs apply pressure and volume constrint to the system
    bool State;
    std::string Type;
    double Tau;
    int updatePeriod;
};
struct STRUC_MCMOVES {  // data structure for turning on and off certain moves
    bool VertexMove;
    bool LinkFlip;
    bool InclusionMove;
    double VertexMoveRate;
    double LinkFlipRate;
    double InclusionMoveRate;
};
struct Parallel_Tempering {  // data structure for turning on and off certain moves
    bool State;
    int PT_steps;
    double PT_minbeta;
    double PT_maxbeta;
};
struct STRUC_ActiveTwoStateInclusion {
    bool state;
    std::string nametype1;
    std::string nametype2;
    double ep1 ;
    double ep2 ;
    double persentage;
    double gama;
};
struct MESH {  // a data structure for a self consistent mesh

private:
    std::vector<vertex>         m_Vertex;
    std::vector<triangle>       m_Triangle;
    std::vector<links>          m_Links;
    std::vector<inclusion>      m_Inclusion;
    Vec3D                       m_Box;
public:
    std::vector <InclusionType> m_InclusionType;
    std::vector <InclusionType*> m_pInclusionType;
    std::vector<vertex*>        m_pAllV;
    std::vector<triangle*>      m_pAllT;
    std::vector<links*>         m_pLinks;
    std::vector<links*>         m_pHL;
    std::vector<links*>         m_pMHL;
    std::vector<inclusion*>      m_pInclusion;
    Vec3D                       *m_pBox;
    
    void GenerateMesh(MeshBluePrint meshblueprint, double kappa, double kappag);
    MeshBluePrint Convert_Mesh_2_BluePrint(MESH *mesh);

};
class State
{
public:
    
	State(std::vector <std::string> argument);
    State();

	 ~State();
    
inline CouplingtoFixedGlobalCurvature *GetGlobalCurvature()                      {return &m_CoupleGCurvature;}
inline SpringPotentialBetweenTwoGroups *Get2GroupHarmonic()                      {return &m_SpringPotentialBetweenTwoGroups;}
inline PositionRescaleFrameTensionCoupling *GetRescaleTension()                  {return &m_RescaleTenCoupl;}
inline CmdVolumeCouplingSecondOrder *GetVolumeCoupling()                         {return &m_VolumeCouplingSecondOrder;}
inline Apply_Osmotic_Pressure *GetOsmotic_Pressure()                         {return &m_Apply_Osmotic_Pressure;}
inline Apply_Constant_Area *GetApply_Constant_Area()                           {return &m_Apply_Constant_Area;}
inline LinkFlipMC *GetMCMoveLinkFlip()                         {return &m_LinkFlipMC;}
inline VertexMCMove *GetMCAVertexMove()                         {return &m_VertexMoveMC;}
inline InclusionMCMove *GetInclusionMCMove()                     {return &m_IncMove;}
inline Restart *GetRestart()                                    {return &m_Restart;}
inline CoupleToWallPotential *GetRigidWallCoupling()                                    {return &m_RigidWallCoupling;}
inline ActiveTwoStateInclusion *GetActiveTwoStateInclusion()                                    {return &m_ActiveTwoStateInclusion;}

public:
    
    bool m_Healthy;   // To check if the input data are read correctly
    bool m_Targeted_State; // Only relavant for Parallel Tempering by 2022; Which state carries the target temparature 
    std::vector <std::string> m_Argument;
    int m_Initial_Step; // initial step
    int m_Final_Step; // final step
    int m_Centering; // box centering frequency; important for pressure coupling
    int m_Seed;       // seed for random number generator
    int m_Total_no_Threads;       // Total no of Threads
    std::string   m_TopologyFile; // name of the topology file, *.top, *.dat *.tsi *.bts
    std::string   m_InputFileName; // name of the topology file, *.top, *.dat *.tsi *.bts
    double m_GaussianRigidity;   //  membrane regidity (Gaussian)
    double m_BendingRigidity;    //  membrane regidity
    double m_Beta ;        // system temperature only applicable if Parallel Tempering Methods is on
    double m_Mem_Spontaneous_Curvature ; //Spontaneous Curvature of the membrane, same effect if the membrane is covered fully by proteins but faster
    double m_MinFaceAngle;          //  minimum angle between the face (smaller will results in error), this is the value of the cos
    double m_R_Vertex;   // Move Vertex  within a box with this size
    double m_R_Box;   // box change within this range
    double m_MinVerticesDistanceSquare; //  minimum distance allowed between two vertices  (smaller will results in error)
    double m_MaxLinkLengthSquare;       //  maximum distance allowed between two nighbouring vertices  (larger will results in error)
    std::string m_GeneralOutputFilename; //  a general file flag for specific run
    std::string m_IndexFileName;            // Name of the index file for group specification
    bool m_IndexFile;                       // to check if the index file is provided
    std::string m_FreezGroupName ;            // Name of a group to be frozen
    STRUC_TRJTSI m_TRJTSI;                  //  an object for tsi file format
    STRUC_TRJBTS m_TRJBTS;                  //  an object for binary  trajectory
    int  m_Display_periodic ;               //  periodic for paraview output
    int m_OutPutEnergy_periodic;            //  periodic for energy file
    Vec3D m_CNTCELL;                    // for domain decomposition
    std::string m_Integrator;               //  Type of integrator (for now only mc exist)
    STRUC_RESTART m_RESTART;                // To check if this is a restart simulation of fresh start
    STRUC_FRAMETENSION m_FrameTension;      // data structure for inputs apply pressure and volume constrint to the system
    STRUC_VOLUME       m_VolumeConstraint ;  // data structure for inputs apply constant area
    STRUC_OSMOTIC      m_STRUC_OSMOTIC;	//
    STRUC_ConstantArea m_STRUC_ConstantArea;
    STRUC_MCMOVES m_MCMove;                 // data structure for turning on and off certain moves
    STRUC_ActiveTwoStateInclusion m_STRUC_ActiveTwoStateInclusion;  // input data to start active two state membrane
    Parallel_Tempering m_Parallel_Tempering; // an object that includes info about Parallel Tempering method that we are applying
    MESH          *m_pMesh;
    Inclusion_Interaction_Map m_inc_ForceField;
    Inclusion_Interaction_Map *m_pinc_ForceField;
    double m_TotEnergy;
    CouplingtoFixedGlobalCurvature  m_CoupleGCurvature;
    SpringPotentialBetweenTwoGroups m_SpringPotentialBetweenTwoGroups;
    PositionRescaleFrameTensionCoupling m_RescaleTenCoupl;
    CmdVolumeCouplingSecondOrder m_VolumeCouplingSecondOrder;
    Apply_Osmotic_Pressure m_Apply_Osmotic_Pressure;
    Apply_Constant_Area m_Apply_Constant_Area;
    ActiveTwoStateInclusion m_ActiveTwoStateInclusion;
    LinkFlipMC m_LinkFlipMC;
    VertexMCMove m_VertexMoveMC;
    InclusionMCMove m_IncMove;
    Restart m_Restart;
    CoupleToWallPotential m_RigidWallCoupling;

private:
    MESH          m_Mesh;
    void ExploreArguments();         // updates variables based on the command line arguments
    void HelpMessage();              // writes a help message
    void ReadInputFile(std::string);    // updates variables based on data in the inputfile
    void WriteStateLog();
   
    


};

#endif
