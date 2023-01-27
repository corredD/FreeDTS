

#include <time.h>
#include "src/Analysis.h"
#include "src/LMatrix.h"
#include "src/WritevtuFiles.h"
#include "src/Nfunction.h"

struct L_CLUSTER {
    
    std::vector<vertex *> Vertex;
    std::vector<int> NCluster;
    std::vector<vertex *> ItsV;
    std::vector<vertex *> NV;
    int checked;
    Vec3D LX;
} ;

Analysis::Analysis(State *pState)
{
    Nfunction f;
    m_TRJ = pState->GetTrajectory();
    std::cout<<m_TRJ.size()<<"size of trj \n";
    m_pState =pState;
    double k= pState->m_BendingRigidity;
    double kg= pState->m_GaussianRigidity;
    int i=0;
    m_totEn = 0;
    m_totVolume = 0;
    m_totArea = 0;
    std::ofstream reponsedata;
    reponsedata.open("response_data.xvg");
    reponsedata<<"@@    step   E      V          A     \n";

    for (std::vector<MeshBluePrint>::iterator it = m_TRJ.begin() ; it != m_TRJ.end(); ++it)
    {
        MESH mesh;
        mesh.GenerateMesh(*it, k, kg);

        UpdateEnergy(mesh);
        UpdateVolume(mesh);
                RemovePBC(mesh);
        Center(mesh);
        std::vector<std::vector<double> > data;
        std::vector<std::string> num_name;
        // include data
        std::vector<vertex*> pV = mesh.m_pAllV;

        std::vector<double> mc; // mean curvature
        std::vector<double> cg; // gaussian curvature
        std::vector<double> en; // energy

        std::string name1="mC";
        num_name.push_back(name1);
        std::string name2="cG";
        num_name.push_back(name2);
        std::string name3="En";
        num_name.push_back(name3);

        for (std::vector<vertex *>::iterator it = pV.begin() ; it != pV.end(); ++it)
        {
            std::vector<double> c = (*it)->GetCurvature();
            mc.push_back(0.5*(c[0]+c[1]));
            cg.push_back(c[0]*c[1]);
            en.push_back((*it)->GetEnergy());
        }
        data.push_back(mc);
        data.push_back(cg);
        data.push_back(en);


        
        

        //=====================================
        
        reponsedata<<i<<"   "<<m_totEn<<"      "<<m_totVolume<<"      "<<m_totArea<<"   \n";
        
        
        //======== writing vtu files
        WritevtuFiles vtu(m_pState);
        std::string fi="conf"+f.Int_to_String(i)+".vtu";
        vtu.Writevtu_Plus_NumberList(num_name, data, mesh.m_pAllV,mesh.m_pAllT,mesh.m_pHL,fi);
        i++;
    }

    

    
    
    
    
}
Analysis::~Analysis()
{
}
void Analysis::UpdateVolume(MESH &mesh)
{
    std::vector<triangle *> pT=mesh.m_pAllT;
    double V=0.0;
    double A=0.0;

    for (std::vector<triangle *>::iterator it = pT.begin() ; it != pT.end(); ++it)
    {
        vertex* pv= (*it)->GetV1();
        double area= (*it)->GetArea();
        Vec3D Norm= (*it)->GetNormalVector();
        Vec3D R (pv->GetVXPos(),pv->GetVYPos(),pv->GetVZPos());
        V+=area*(R.dot(Norm,R))/3.0;
        A+=area;
    }
    m_totVolume = V;
    m_totArea = A;

}
void Analysis::UpdateEnergy(MESH &mesh)
{
    std::vector<vertex*> pV = mesh.m_pAllV;
    std::vector<links *> pL = mesh.m_pHL;
    std::vector<triangle *> pT=mesh.m_pAllT;
    Vec3D *pBox=mesh.m_pBox;

    //========== Update the Mesh geometry variables ===============
    //======== Prepare the trinagles: calculate area and normal vector
    for (std::vector<triangle *>::iterator it = pT.begin() ; it != pT.end(); ++it)
        (*it)->UpdateNormal_Area(pBox);
    //===== Prepare links:  normal vector and shape operator
    for (std::vector<links *>::iterator it = pL.begin() ; it != pL.end(); ++it)
    {
        (*it)->UpdateNormal();
        (*it)->UpdateShapeOperator(pBox);
    }
    //======= Prepare vertex:  area and normal vector and curvature
    for (std::vector<vertex *>::iterator it = pV.begin() ; it != pV.end(); ++it)
        Curvature P(*it);
    
    Energy En (m_pState->m_pinc_ForceField);
    m_totEn = En.TotalEnergy(pV,pL);

}
void Analysis::Center(MESH &mesh)
{
    std::vector<vertex*> pV = mesh.m_pAllV;
    Vec3D Box = *(mesh.m_pBox);
    Vec3D CM;

    for (std::vector<vertex *>::iterator it = pV.begin() ; it != pV.end(); ++it)
    {
        Vec3D Pos((*it)->GetVXPos(),(*it)->GetVYPos(),(*it)->GetVZPos());
        
        CM = CM+Pos*(1/double(pV.size()));
    }
    for (std::vector<vertex *>::iterator it = pV.begin() ; it != pV.end(); ++it)
    {
        Vec3D Pos((*it)->GetVXPos(),(*it)->GetVYPos(),(*it)->GetVZPos());
        (*it)->NOPBCUpdatePos(Pos-CM+Box*0.5);
    }
    
    
}
void Analysis::RemovePBC(MESH &mesh)
{



    std::vector<vertex*> pV = mesh.m_pAllV;
    std::vector< L_CLUSTER > AllCluster;

    LMatrix VerMat(pV.size(),pV.size(),'I');
    for (int n=0;n<pV.size();n++)
        VerMat(n,n) = 1;
    
    

    
    for (std::vector<vertex *>::iterator it1 = pV.begin() ; it1 != pV.end(); ++it1)
    {
            int id1=(*it1)->GetVID();
            std::vector <vertex *> NV = (*it1)->GetVNeighbourVertex();
            int connect = 0;

            for (std::vector<vertex *>::iterator it3 = NV.begin() ; it3 != NV.end(); ++it3)
            {
                 connect = 0;
                int id2 = (*it3)->GetVID();

                    double dx =(*it1)->GetVXPos()-(*it3)->GetVXPos();
                    double dy =(*it1)->GetVYPos()-(*it3)->GetVYPos();
                    double dz =(*it1)->GetVZPos()-(*it3)->GetVZPos();
                    
                    
                    double dist=dx*dx+dy*dy+dz*dz;
                    //// check the distance
                    
                    if(dist<3.001)
                        connect = 1;
                if(connect == 1)
                    VerMat(id1,id2) = connect;
                }

        }


for (int n=0;n<pV.size();n++)
{
    for (int m=n+1;m<pV.size();m++)
    {
        if(VerMat(n,m)!=0)
        {
            for (int i=0;i<pV.size();i++)
            {
                if(VerMat(m,i)==0)
                    VerMat(m,i)=VerMat(n,i);
                    VerMat(n,i)=0;
                    
                    }
            break;
        }
    }
}
    
    int clid = 0 ;
    int Vsize = pV.size();
    for (int n=0;n<Vsize;n++)
    {
        int cluster = 0;
        //  std::cout<<clid<<"  "<<n<<" here 1 wrif[vm, \n";
        
        for (int m=0;m<pV.size();m++)
        {
            if(VerMat(n,m)!=0)
            {
                cluster++;
                break;
            }
        }
        
        std::vector<vertex *> TV;
        if(cluster!=0)
        {
            
            // TV.push_back(m_pAllV.at(n));
            for (int m=0;m<pV.size();m++)
            {
                if(VerMat(n,m)!=0)
                {
                    (pV.at(m))->UpdateGroup(clid);
                    //  (m_pAllV.at(m)->GetInclusion())->UpdateType("P"+f.Int_to_String(clid),clid);
                    TV.push_back(pV.at(m));
                    
                }
            }
            
            L_CLUSTER CL;
            CL.Vertex =TV;
            CL.checked = 0;
            Vec3D LX;
            CL.LX=LX;
            AllCluster.push_back(CL);
            
            clid++;
        }
    }
    std::cout<<"System contains "<<AllCluster.size()<<"  CLusters \n";
    for (std::vector<L_CLUSTER>::iterator it = AllCluster.begin() ; it != AllCluster.end(); ++it)
    {
        //  (*it).checked = 1;
        std::vector <vertex *> CV = (*it).Vertex;
        for (std::vector<vertex*>::iterator it1 = CV.begin() ; it1 != CV.end(); ++it1)
        {
            std::vector <vertex *> NV = (*it1)->GetVNeighbourVertex();
            for (std::vector<vertex*>::iterator it2 = NV.begin() ; it2 != NV.end(); ++it2)
            {
                if((*it1)->GetGroup() == (*it2)->GetGroup())
                {
                    
                }
                else
                {
                    ////====
                    int clusterid1 = (*it1)->GetGroup();
                    int clusterid2 = (*it2)->GetGroup();
                    L_CLUSTER CL2 = AllCluster.at(clusterid2);
                    bool isalready = false;
                    std::vector<int> NCluster1 = (*it).NCluster;
                    std::vector<int> NCluster2 = CL2.NCluster;
                    std::vector<vertex *> ItsV1 = (*it).ItsV;
                    std::vector<vertex *> NV1 = (*it).NV;
                    std::vector<vertex *> ItsV2 = CL2.ItsV;
                    std::vector<vertex *> NV2 = CL2.NV;
                    for (std::vector<int>::iterator itint = NCluster1.begin() ; itint != NCluster1.end(); ++itint)
                    {
                        if(clusterid2==(*itint))
                        {
                            isalready =true;
                            break;
                        }
                    }
                    if(isalready==false)
                    {
                        ItsV1.push_back(*it1);
                        NV1.push_back(*it2);
                        ItsV2.push_back(*it2);
                        NV2.push_back(*it1);
                        (*it).NV=NV1;
                        (*it).ItsV=ItsV1;
                        CL2.NV=NV2;
                        CL2.ItsV=ItsV2;
                        NCluster1.push_back(clusterid2);
                        NCluster2.push_back(clusterid1);
                        CL2.NCluster=NCluster2;
                        (*it).NCluster=NCluster1;
                    }
                }
            }
        }
    }
    
    //==
    std::vector<vertex> m_OLDV;
    
    for (std::vector<vertex*>::iterator it = pV.begin() ; it != pV.end(); ++it)
    {
        m_OLDV.push_back(*(*it));
    }
    Vec3D m_OLDBox = *(mesh.m_pBox);
    //  m_Box = m_Box*10;;
    (AllCluster.at(0)).checked=1;
    
    //  Translate(m_pAllV, (m_OLDBox*5));
    
    srand(88888);
    int i=0;
    
    while (true)
    {
        int breaking = true;
        for (std::vector<L_CLUSTER>::iterator it = AllCluster.begin() ; it != AllCluster.end(); ++it)
        {
            
            //  std::cout<<i++<<"  "<<(*it).checked<<"\n";
            
            if((*it).checked==0)
            {
                breaking = false;
                break;
            }
        }
        
        if(breaking==true)
            break;
        
        
        int rng= rand() % (AllCluster.size());
        L_CLUSTER CL = AllCluster.at(rng);
        std::vector<vertex *> Vertex = CL.Vertex;
        std::vector<int> NCluster = CL.NCluster;
        std::vector<vertex *> ItsV = CL.ItsV;
        std::vector<vertex *> NV = CL.NV;
        
        bool move = false;
        int nid=0;
        int id=0;
        
        
        
        for (std::vector<int>::iterator it = NCluster.begin() ; it != NCluster.end(); ++it)
        {
            if(AllCluster.at((*it)).checked==1)
            {
                nid=(*it);
                move = true;
                break;
            }
            id++;
        }
        if(move==true && AllCluster.at(rng).checked==0)
        {
            i++;
            
            AllCluster.at(rng).checked=1;
            if(NCluster.at(id)!=nid)
                std::cout<<"error \n";
            
            int vid1=(ItsV.at(id))->GetVID();
            int vid2=(NV.at(id))->GetVID();
            Vec3D TX;
            
            
            double dx = (m_OLDV.at(vid1)).GetVXPos()-(m_OLDV.at(vid2)).GetVXPos();
            double dy = (m_OLDV.at(vid1)).GetVYPos()-(m_OLDV.at(vid2)).GetVYPos();
            double dz = (m_OLDV.at(vid1)).GetVZPos()-(m_OLDV.at(vid2)).GetVZPos();
            Vec3D DX(dx,dy,dz);
            
            
            for (int s=0;s<3;s++)
            {
                if(fabs(DX(s))<m_OLDBox(s)/2.0)
                {
                    TX(s)=0;
                }
                else
                {
                    if((DX(s))<0)
                    {
                        TX(s)=m_OLDBox(s);
                    }
                    else
                    {
                        TX(s)=-m_OLDBox(s);
                        
                    }
                }
            }
            
            
            
            Vec3D LX = (AllCluster.at((NV.at(id))->GetGroup())).LX;
            
            Vec3D LX2 = LX+TX;
            (AllCluster.at(rng)).LX=LX2;
            Translate(Vertex, LX2);
            
            
        }
        
        
    }
 
}
void Analysis::Translate(std::vector<vertex*> v, Vec3D L)
{
    for (std::vector<vertex*>::iterator it = v.begin() ; it != v.end(); ++it)
    {
        Vec3D X((*it)->GetVXPos(),(*it)->GetVYPos(),(*it)->GetVZPos());
        
        Vec3D X2=X+L;
        (*it)->NOPBCUpdatePos(X2);

    }
}
