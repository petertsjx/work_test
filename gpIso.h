#ifndef _gpIso_h
#define _gpIso_h

class puMgr;
class puRectBin;
class gpIsocell;
class gpIsoGroup;
typedef typedsList<gpIsocell>gpIsoCellList;
typedef typedsList<gpIsoGroup>gpIsoGroupList;

class gpIsoMgr{
    friend class gpIsoCellIter;
    friend class gpIsoGroupIter;

public:
    gpIsoMgr(puMgr *);
    ~gpIsoMgr();

    void
    Move();
    UInt GetNumIsoCells() const;
    void PostProcess();
    void DumpIsoCells() const;
    void DrawCornerGroups() const;
    void DumpCornerGroups() const;
    void DrawSideGroups() const;
    void DumpSideGroups() const;
    void DrawCells() const;
    puMgr *GetPuMgr() const{
        return pmgr;
    }

    gpIsoGroup *GocIsoGroup(const guPoint *,int dirx, int dirY,int rgnIdx);
    gpIsoGroup *GocIsoSideGroup(const int, int dirx, int dirY,int rgnIdx,bool axis);
    puRectBin * GetBlockageBin() const{
        return _blkBin;
    }
    gpIsoCellList *GetIsoCellList() const{
        return _isoCellList;
    }
    gpIsoGroupList*GetIsoGroupList() const{
        return _isoGrpList;
    }

private:
    void Init();
    void AssignGroup();
    void AssignSideGroup();
    void AnalyzeCornerGroups();
    void AssignEdgeGroups();
    void AnalyzeSideGroups();
    bool AssignEdgeIsoCell(gpIsoCell *);
    void PlaceCornerGroups();
    void BuildBlockageBin();
    gpIsoCelLList * NewIsoCellList();
    gpIsoGroupList *NewIsoGroupList();
    gpIsoCell *NewIsoCell(dbInst *);
    gpIsoGroup*NewIsoGroup(const guPoint *,int dirx, int dirY,int rgnIdx);
    void RecycleIsoCell(gpIsoCell *);
    void RecycleIsoGroup(gpIsoGroup *);
    void RecycleIsoCellList(gpIsoCellList *);
    void RecycleIsoGroups();

private:
    puMgr* _pmgr;
    utMemPool *_pool;
    utobjPool *_lsPool;
    utobjPool *_lePool;
    utobjPool *_isoCellPool;
    utobjPool *_isoGrpPool;
    // list of all Iso cells,initialized in Init()
    gpIsoCellList *_isoCellList;
    // list of Iso groups, built and recycled at the end of each Move()
    gpIsoGroupList *_isoGrpList;
    puRectBin *blkBin;
};

class gpIsoGroup 
public:
    gpIsoGroup(gpIsoCellList *,const guPoint *，int dirx,int dirY);
    void Analyze(gpIsoMgr *);
    void PlaceIsoCells();
    void CalsideGroupPt(int);
    void SetPtByAxis(int,int);
    int GetX() const{
        return _pt.Getx();
    }
    int GetY() const{
        return _pt.GetY();
    }
    int GetDirX() const{
        return _dirX;
    }
    int GetDirY() const{
        return _dirY;
    }
    void SetMinRow(int v){
        minRow =v;
    }
    int GetMinRow() const{
        return _minRow;
    }
    void SetMaxRow(int v){
        maxRow=V;
    }
    int GetMaxRow() const {
        return _maxRow;
    }
    int GetMinWidth() const{
        return _wl;
    }
    int GetMaxWidth() const{
        return _w2;
    }
    bool IsCornerGroup() const;
    void AddIsoCell(gpIsoCell *);
    void SetAnalyzed(bool v){
        analyzed=v;
    }
    bool IsAnalyzed() const {
        return _analyzed;
    }
    bool IsSideGroup() const {
        return _isside;
    }






    
