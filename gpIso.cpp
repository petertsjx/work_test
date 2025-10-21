
gpIsoMgr::gpIsoMgr（puMgr *pmgr){
    memset（this,0,sizeof(gpIsoMgr));
    pmgr=pmgr;
    Init();
}

gpIsoMgr::~gpIsoMgr(){
    RecycleIsoCellList(GetIsoCellList());
    RecycleIsoGroups();
    delete GetIsoGroupList();
    delete _pool;
    if（blkBin）{
        delete blkBin;
    }
}

void gpIsoMgr::Init(){
    _pool=new utMemPool("gpIsoMgr");
    lePool=_pool->NewobjPool(sizeof(utSListEntry));
    isoCellPool=_pool->NewobjPool(sizeof(gpIsoCell));
    isoGrpPool =_pool->NewObjPool(sizeof(gpIsoGroup));
    isoCellList= NewIsoCellList();
    isoGrpList =NewIsoGroupList();
    puMgr* pmgr =puMgr::GetMgr(_pmgr->GetTopModule());
    puGPInstIter gpInstIter;
    dbInst* inst;
    for (gpInstIter.Begin(pmgr);!gpInstIter.End(inst);gpInstIter++){
        if(!inst->IsIsolationCell()&&!inst->IsLevelshifter()&&!inst->IsRetentionCell()){
            continue;
        }
    }
#ifdef DEBUG ISO
    pu::IsDebug(inst,"gpIsoCell");
#endif
    gpIsoCell *isoCell=NewIsoCell(inst);
    if(isoCell->GetAnchor()){
        GetIsoCellList()->AddTail(isoCell);
    }
    else{
        RecycleIsocell(isocell);
    }
    if(GetNumIsoCells()>0){
        GetIsoCellList()->Reverse();
        BuildBlockageBin();
    }
    if(puParam::IsVerbosePrivateLic()){
        cout<<“found <<GetNumIsoCells()<<"LS/IsO cells"<<endl;
    }
}

gpIsoCellList *gpIsoMgr::NewIsoCellList(){
    return new gpIsoCellList(lePool);
}

void gpIsoMgr::RecycleIsoCellList(gpIsoCellList *isoCellList){
    delete isoCellList;
}

void gpIsoMgr::RecycleIsoCellList(gpIsoCellList *isoCellList){
    delete isoCellList;
}

gpIsoGroupList *gpIsoMgr::NewIsoGroupList(){
    return new gpIsoGroupList(_lePool);
}

void gpIsoMgr::RecycleIsoGroups(){
    gpIsoGroupIter isoGroupIter;
    gpIsoGroup *
    group;
    for (isoGroupIter.Begin(this);!isoGroupIter.End(group);isoGroupIter++){
        RecycleIsoGroup(group);
        isoGroupIter.Removeobj();
    }
}

gpIsoCell *gpIsoMgr::NewIsoCell(dbInst *inst){
    return new(_isoCellPool->Allocobj())gpIsocell(inst);
}

void gpIsoMgr::RecycleIsoCell(gpIsoCell *isoCell){
    isoCellPool->Recycleobj(isoCell);
}

gpIsoGroup *gpIsoMgr::NewIsoGroup(const guPoint *pt,int dirx,int dirY,int rgnIdx){
    gpIsoGroup *group=new(_isoGrpPool->Allocobj())
    gpIsoGroup(NewIsoCelLList(),pt,dirX,dirY);
    GetIsoGroupList()->AddTail(group);
    group->SetGrpIdx(GetIsoGroupList()->GetobjCount());
    group->SetRgnIdx(group->IsCornerGroup()?rgnIdx:O);
    return group;
}

void gpIsoMgr::RecycleIsoGroup(gpIsoGroup *isoGrp){
    RecycleIsoCellList(isoGrp->GetIsoCellList());
    isoGrpPool->RecycleObj(isoGrp);
}

UInt gpIsoMgr::GetNumIsoCells() const {
    return GetIsoCellList()->GetobjCount();
}

void gpIsoMgr::Move(){
    if (puParam::GetIsolationCellPlacementMethod()==2){
        AssignGroup();
        AnalyzeCornerGroups();

        if(puParam::IsDebugIso()){
            cout <<"AnalyzeCornerGroupsi()"<<endl;
            DrawCornerGroups();
            DumpcornerGroups();
            utPause();
            pu::clear();
        }

        AssignEdgeGroups();
        AnalyzeCornerGroups();

        if(puParam::IsDebugIso()){
            cout <<"AnalyzeCornerGroups2()"<<endl;
            DrawCornerGroups();
            DumpCornerGroups();
            utPause();
        }

        PlaceCornerGroups();

        if(puParam::IsDebugIso()){
            cout<<"PlaceIso()"<<endl;
            DrawCells();
            utPause();
            pu::Clear();
        }
        RecycleIsoGroups();
    }else if(puParam::GetIsolationCellPlacementMethod()==1){
        gpIsocellIter isoCellIter;
        gpIsoCell *cell;
        for (isoCellIter.Begin(this);!isoCellIter.End(cell);isoCellIter++){
            cell->MoveAnchor();
            cell->Move();
        }
    }
    pu::DumpUsage("isoMoveEnd");
}

void gpIsoGroup::CalSideGroupPt(int i){
    gpIsoCellIter cIter;
    gpIsoCell *
    cell;
    int minCoord=INT MAX;
    int maxCoord=INT MIN;
    int sidecoord=0;
    int average=0;
    int num=0;
    for(cIter.Begin(this);!cIter.End（cell);cIter++)
        puInst *pinst =pu::GetInst(cell->GetInst());
        num +=1;
    if(GetAxis()==false){//x
        if(pinst->GetCenterX()<mincoord){
            minCoord =pinst->GetCenterx();
            sideCoord =pinst->GetCenterY();
        }
        if(pinst->GetCenterX()>maxCoord){
            maxCoord=pinst->GetCenterX();
        }
        average+=pinst->GetCenterX();
    }else if(GetAxis()==true){//y
        if(pinst->GetCenterY()<minCoord){
            minCoord =pinst->GetCenterY();
            sideCoord =pinst->GetCenterx();
        }
        if(pinst->GetcenterY()>maxcoord){
            maxCoord =pinst->GetCenterY();
            average+=pinst->Getcenterx();
        }
    }else if(GetAxis()==true){//y
        if(pinst->GetCenterY()<mincoord){
            minCoord =pinst->GetCenterY();
            sideCoord = pinst->GetCenterX();
        }
        if(pinst->GetCenterY()>maxCoord)
        {
            maxCoord=pinst->GetCenterY();
        }
        average+=pinst->GetCenterX();
    }
    _groupLength=maxCoord-minCoord;
    if(i==0){//choose min coord asgroup corner;
        SetPtByAxis(minCoord+puParam::GetIsolationCellPlacementPostProcessCornerDisplacement(),sideCoord);
    }else if(i==1){//toDo
        sideWidth=maxCoord-minCoord;
        SetPtByAxis(minCoord+(groupAverageCenter-(minCoord+maxCoord)/2),sideCoord);
    }
}

void gpIsoGroup::SetPtByAxis(int x,int y){
    if(GetAxis()==false)
    {//x
        _pt.Setx(x);
        _pt.SetY(y);
    }else{//y
        pt.Setx(y);
        _pt.SetY(x);
    }
}

void gpIsoMgr::PostProcess(){
    AssignsideGroup();
    AnalyzesideGroups();
    if(puParam::IsDebugIso()){
        cout << "AnalyzesideGroupsl()"<< endl;
        DrawSideGroups();
        DumpSideGroups()
        utPause();
        pu::clear();
    }
    PlaceCornerGroups();
    if(puParam::IsDebugIso())
    {
        cout << "PlaceIso()"<<endl;
        Drawcells();
        utPause();
        pu::clear();
    }
    RecycleIsoGroups();
}

void gpIsoMgr::AssignSideGroup(){
    puBatchDraw *xx = NULL;
    if(puParam::IsDebugIso()){
        cout << "AssignsideGroup()" << endl;
        xx = new puBatchDraw;
    }
    gpIsoCellIter iter;gpIsoCell *isoCell;
    for(iter.Begin(this);!iter.End(isocell);iter++){
        isocell->MoveAnchor();
        puInst *pinst = pu::GetInst(isocell->GetInst());
        guPoint pt0;// original centerpto.Set(pinst->Getcenter());
        guPoint ancLoc;//center of anchorisocell->GetAnchorLoc(&ancLoc);pinst->Shift(&ancLoc);pinst->SnapctrIntoRegion();
        guPoint ptl;// center after snappingptl.set(pinst->Getcenter());
        int dx, dy;
        ptl.GetDisplacement(&ancLoc, dx,dy);
        bool isSide=false;
        int dirX=0,dirY=0,sidecoord=0;
        bool axis=false;
        dbPowerDomain *pd=isoCell->GetInst()->GetPowerDomain();
        cout<<"pdName:"<<isoCell->GetInst()->GetPowerDomainName()<<endl;
        int x1=pd->GetBndy()->GetBBox()->GetLeft();
        int x2 =pd->GetBndy()->GetBBox()->GetRight();
        int yl = pd->GetBndy()->GetBBox()->GetBottom();
        int y2 =pd->GetBndy()->GetBBox()->GetTop();
        cout<<"x1:"<<pu: :GetUserCoord(x1)<<" x2: "<<pu::GetUserCoord(x2)<<" y1: "<<pu::GetUsercoord(y1)<<" y2: "
        if((dx ==0)||(dy == 0)){
            pinst->SnapIntoRegion();
            if((dx==0)&&(dy>0))
            {
                sideCoord = pinst->GetCenterY();
                axis=false;//x
                dirX=dirY=l;isSide=true;
            }else if((dx==0)&&(dy<0)){
                sideCoord = pinst->GetcenterY();
                axis=false://x
                dirX=l,dirY=-1;
                isSide=true;
            }else if((dy==0)&&(dx<0)){
                sideCoord=pinst->GetCenterx();
                axis=true;//y
                dirX=-1,dirY=1;
                isSide=true;
            }
            cout<<"sidecoord:"<<pu::GetUserCoord(sideCoord)<<"axis:"<<pu::GetUserCoord(axis)<<endl;
            gpIsoGroup *grp= GocIsoSideGroup(sideCoord,dirx,dirY,pinst->GetRegion()->GetIdx(),axis);
            grp->AddIsoCell(isocell);
            isoCell->SetInitLoc(&pto);
            isoCell->SetDist(ptl.GetxyDistance(&pto));
            if(isSide){
                grp->SetSideGroup(true);
                grp->SetRgnIdx(isSide?pinst->GetRegion()->GetIdx():0);
            if(puParam::IsDebugIso()&&grp->IsSideGroup()){
                pu::DrawLine（&pt0,&pt1）;
            }
        }
    }
    if(puParam::IsDebugIso())
    {   
        delete xx;
        utPause();
    }
}
void gpIsoMgr::AssignGroup(){
    puBatchDraw *xx=NULL;
    if(puParam::IsDebugIso()){
        cout <<"AssignGroup()"<<endl;
        xx=new puBatchDraw;
    }

    gpIsoCellIter iter;
    gpIsocell * isoCell;
    for(iter.Begin(this);!iter.End(isoCell);iter++) {
        isoCell->MoveAnchor();

        puInst *pinst =pu::GetInst(isoCell->GetInst());

        guPoint pt0;// original center
        pt0.Set(pinst->GetCenter());

        guPoint ancLoc;// center of anchor

        isoCell->GetAnchorLoc(&ancLoc);
        pinst->Shift(&ancLoc);
        pinst->SnapctrIntoRegion();

        guPoint pt1;// center after snapping
        pt1.Set(pinst->GetCenter());

        int dx，dy;
        ptl.GetDisplacement(&ancLoc,dx,dy);

        guPoint corner;// corner point
        corner.Set(pt1);
    }
    int dirx,dirY;//1:gohigh;-1:golow
    if((dx==0）11（dy==0)){
        pinst->SnapIntoRegion();
        corner.Set(INT _MIN,INT MIN);
        dirX=dirY=0;
    }
    else if(dx>θ){//leftside
        if(dy>0){//BLcorner
            dirx=dirY=1;
        else{//dy<0:TLcorner
            dirx=1;
            dirY=-1;
        }
        elseif(dy>0){//dx<0:BRcorner
            dirX=-1;
            dirY=1;
        }
        else{//dx<0,dy<0:TRcorner
            dirx=dirY=-1;
        }
    }

    gpIsoGroup *grp =GocIsoGroup(&corner,dirX,dirY,pinst->GetRegion()->GetIdx());
    grp->AddIsoCell(isoCell);
    isoCell->SetInitLoc(&pt0);
    isoCell->SetDist(ptl.GetxyDistance(&pto));
    if(puParam::IsDebugIso()&&grp->IsCornerGroup()){
        pu::DrawLine(&pt0,&pt1);
    }
    if(puParam::IsDebugIso()){
        delete xx;
        utPause();
    }
}

gpIsoGroup *gpIsoMgr::GocIsoGroup(const guPoint *pt, int dirX, int dirY, int rgnIdx){
    gpIsoGroupIter isoGroupIter;
    gpIsoGroup *group;
    for (isoGroupIter.Begin(this);!isoGroupIter.End(group); isoGroupIter++){
        if(*group->GetPoint()==*pt){
            if((group->GetRgnIdx()==0)||(group->GetRgnIdx()== rgnIdx)){
                return group;
            }
        }
    }
    return NewIsoGroup(pt, dirx, dirY, rgnIdx);
}

void gpIsoMgr::AnalyzeSideGroups(){
    gpIsoGroupIter isoGroupIter;
    gpIsoGroup * group;
    for (isoGroupIter.Begin(this);!isoGroupIter.End(group); isoGroupIter++){
        if(group->IssideGroup()&& !group->IsAnalyzed()){
            group->Analyze(this);
        }
    }
}

gpIsoGroup *gpIsoMgr::GocIsosideGroup(const int sidecoord, int dirX,int dirY,int rgnIdx,boolaxis)
{
    guPoint *pt = new guPoint(0,0);
    gpIsoGroupIter isoGroupIter;
    gpIsoGroup *group;
    for (isoGroupIter,Begin(this);!isoGroupIter.End(group); isoGroupIter++){
        if ((group->IssideGroup())&&(group->Getsidecoord()==sideCoord)&&(group->GetAxis()==axis)){
            if((group->GetRgnIdx()==0)||(group->GetRgnIdx()== rgnIdx)){
                return group;
            }
        }
    }
    gpIsoGroup *grp=NewIsoGroup(pt, dirX, dirY, rgnIdx);grp->SetAxis(axis);
    grp->setsideCoord(sidecoord);
    return grp;
}

void gpIsoMgr::AnalyzeCornerGroups(){
    gpIsoGroupIter isoGroupIter;
    gpIsoGroup *group;
    for (isoGroupIter.Begin(this);!isoGroupIter.End(group); isoGroupIter++){
        if(group->IsCornerGroup()&& !group->IsAnalyzed()){
            group->Analyze(this);
        }
    }
}

void gpIsoMgr::AssignEdgeGroups(){
    gpIsoGroupIter gIter;gpIsoGroup * grp;
    for(gIter.Begin(this);!gIter.End(grp);gIter++){if(grp->IsCornerGroup())
        continue;
    }
    gpIsoCellIter cIter;gpIsocell*isocell;
    for(cIter.Begin(grp);!cIter.End(isocell);cIter++){
        if(AssignEdgeIsocell(isocell)){
            cIter.Remove0bj();
        }
    }
}

bool gpIsoMgr::AssignEdgeIsocell(gpIsocell *isocell){
    gpIsoGroup *minGrp =NULL;
    intminDist =INT MAX;
    gpIsoGroupIter iter;gpIsoGroup *grp;
    for(iter.Begin(this);!iter.End(grp);iter++){
        if(!grp->IsCornerGroup())continue;
    }
    int d=isocell->GetLoc()->GetXyDistance(grp->GetPoint());
    if((d<= grp->GetMaxWidth())&&(d< minDist)){
        minDist =d;
        minGrp = grp;
    }
    if(minGrp){
        minGrp->AddIsocell(isocell);
        minGrp->setAnalyzed(false);
        isocell->AddDist(minDist);
        return true;
    }
    return false;
}

void gpIsoMgr::DumpIsocells()const {
    cout << GetNumIsocells()<<" Iso cells\n";
    gpIsoCellIter iter;gpIsocell *cell;
    for (iter.Begin(this);!iter.End(cell); iter++){
        cout << cell->GetInst()->GetHierName()<< ""<< pu::Point2Str(cell->GetLoc())<< endl;
    }
}

void gpIsoMgr::DrawCells() const
{
    puBatchDraw xx;
    gpIsoCellIter iter;
    gpIsocell *cell;
    for(iter.Begin(this);!iter.End(cell);iter++)
        pu::DrawInst(cell->GetInst(),1);
}

void gpIsoMgr::DumpSideGroups() const
{ 
    gpIsoGroupIter iter;
    gpIsoGroup *
    grp;
    for (iter.Begin(this);!iter.End(grp);iter++)
        if(grp->IsSideGroup()){
            grp->Dump();
        }
    }
}
void gpIsoMgr::DrawSideGroups()
{
    const puBatchDraw xx;
    gpIsoGroupIter iter;
    gpIsoGroup* grp;
    for(iter.Begin(this);!iter.End(grp);iter++){
        if(grp->IsSideGroup()){
            grp->Draw();
        }
    }
}

void gpIsoMgr::DrawCornerGroups() const{
    puBatchDraw xx;
    gpIsoGroupIter iter;
    gpIsoGroup* grp;
    for (iter.Begin(this);!iter.End(grp);iter++){
        if(grp->IscornerGroup()){
            grp->Draw();
        }
    }
}

void gpIsoMgr::DumpcornerGroups()const {
    gpIsoGroupIter iter;
    gpIsoGroup * grp;
    for (iter.Begin(this);!iter.End(grp); iter++){
        if(grp->IsCornerGroup()){
            grp->Dump();
        }
    }
}

void gpIsoMgr::BuildBlockageBin()
{
    blkBin =new puRectBin("isoBlkBin",_pmgr->GetBndy()->GetBBox(),
    64*pu::GetDefaultCellHeight(),0,1);

    guRectList bndyBlk,bndyBBox;
    bndyBBox.AddRect(_pmgr->GetBndy()->GetBBox());
    bndyBlk.SetNot(&bndyBBox,_pmgr->GetBndy());
    _blkBin->Add(&bndyBlk);
#ifdef DEBUG BLOCKAGE
    puBatchDraw xx;
    pu::DrawRectList(&bndyBlk,1);
#endif
    dbModuleInstIter iter;
    dbInst* inst;
    guRectList  macBndy;
    for (iter.BeginAllLeaf(_pmgr->GetModule());!iter.End(inst);iter++){
        if(pu::IsDbMacro（inst)){
            inst->GetPlBndy(&macBndy);
            blkBin->Add(&macBndy);
#ifdef DEBUG BLOCKAGE
             pu::DrawRectList(&macBndy,1);
#endif
        }
    }
}

gpIsoCell::gpIsoCell(dbInst *inst){
    _inst=inst;
    _anchor =FindAnchorPin();
    if(anchor == NULL)// will be thrown away
        return;
    guRectbbox;
    inst->GetPlBndyBBox(&bbox);
    float al=bbox.GetArea()*puParam::GetIsolationCellAreaScale();
    float a2=pu::GetInst(inst)->GetCurrArea();
    SetArea(UTMAX(al,a2));

    int cellH =pu::GetDefaultCellHeight();
    SetHeight((bbox.GetHeight()+cellH-1)/cellH);
    SetWidth(utCeil(GetArea()/cellH/GetHeight()));

    dbInst *aInst =_anchor->GetInst();
    if (aInst && pu::IsDbUnplaced(aInst)){
        aInst->SetPlaceStatus(db::PLACED);
    }
}

dbPin *gpIsoCell::FindAnchorPin() const{
    int bestScore =0;
    dbPin *bestAnchor=NULL;
    dbModule* topMod=pu::GetTopModule( inst);
    dbInstPinIter iter;
    dbPin * pin;
    for(iter.Begin(inst);!iter.End(pin);iter++){
        int score;
        dbPin *pin2= FindAnchorPin(pin,topMod,score);
        if (pin2&&（score>=bestScore){
            bestscore = score;
            bestAnchor =pin2;
        }
        return bestAnchor;
    }
}

dbPin *gpIsoCell::FindAnchorPin(dbPin *pin,dbModule *mod,int &score){
    const score=0;
    if(pin->IsIsolationcellEnable()Il pin->IsLevelshifterEnable())
        return NULL;

    dbNet *net=pin->GetFlattenNet(mod);
    if(net==NULL || net->IsPG())
        return NULL;

    dbPowerDomain *dom = pu::GetPowerDomain(pin);
    dbNetPinIter iter;
    dbPin*  pin2;
    if(pin->IsSigSource()) {
        for(iter.BeginAllTerm(net,mod);!iter.End(pin2);iter++)
        {
            if(pin2->IsSigSource())
                continue;
            if(pu::GetPowerDomain(pin2)!=dom) {
                ++score;
                return pin2;
            }
        }
        return NULL;
    }else{
        ++score;
        for(iter.BeginAllTerm(net,mod);!iter.End(pin2);iter++)
            if (!pin2->IsSigSource())
                continue;
            if(pu::GetPowerDomain(pin2) !=dom){
                score+=3;
                return pin2;
            }
            return NULL;
        }
    }
}

void gpIsoCell::MoveAnchor() const{
    dbInst *inst =GetAnchor()->GetInst();
    if(inst==NULL)
        return;
    puInst *pinst=pu::GetInst(inst);
    if(!pinst->IsPlaced())
        return;
    int x,y,dx,dy;
    inst->GetPlCenter(x,y);
    pinst->GetCenter()->GetDisplacement(x,y,dx,dy);
    inst->Shift(dx,dy);
}

void gpIsoCell::GetAnchorLoc(guPoint *pt) const
{
#if 0
    dbInst *aInst =GetAnchor()->GetInst();
    if(aInst){
        puInst *pinst= pu::GetInst(aInst);
        if(pinst->IsPlaced()){
            pt->Set(pinst->Getcenter());
            return;
        }
#endif
    }
    GetAnchor()->GetoneLocation(pt);
}

const guPoint *gpIsocell::GetLoc() const{
    return pu::GetInst(inst)->GetCenter();
}

int gpIsoCell::GetPlWidth() const{
    return pu::GetInst(_inst)->GetWidth();
}

int gpIsoCell::CompareDist(const void *a, const void *b,const void *c)
{
    const gpIsocell *cl = static_cast<const gpIsocell *>(a);
    const gpIsocell *c2= static cast<const gpIsocell *>(b);
    if(c1->GetDist()<c2->GetDist())
        return -1;
    else if(c2->GetDist()<c1->GetDist())
        return 1:
    return 0;
}

gpIsoGroup::gpIsoGroup(gpIsoCellList *isoCellList,const guPoint *pt,int dirx,int dirY){
    isoCellList=isoCellList;
    if(pt){
        pt.Set(pt);
    }else{
        _pt.Set(INT_MIN,INT_MIN);
        dirX=dirx;
        dirY=dirY;
    }
}
void gpIsoGroup::AddIsoCell(gpIsoCell *isoCell){
    GetIsoCellList()->AddTail(isoCell);
}
bool gpIsoGroup::IsCornerGroup() const{
    return ((GetX())!=INT MIN)&&(GetY()!=INT MIN);
}

void gpIsoGroup::Analyze(gpIsoMgr *mgr){
    totArea =0.0;
    gpIsoCellIter cIter;
    gpIsoCell *cell;
    for(cIter.Begin(this);!cIter.End(cell);cIter++){
        totArea += cell->GetArea();
    }
    double r= puParam::GetIsolationCellPlacementAspectRatio();
    double w=sqrt( totArea /(2.0*r- 1.0));//doublel=totArea/w;double h=w*r;
    puRegion *rgn=nullptr;SetBoxes(utceil(w),utceil(h));
    if(mgr->GetPuMgr()->GetRegionMgr())
    {
        rgn =mgr->GetPuMgr()->GetRegionMgr()->GetRegion( rgnIdx);
    }
    else{
        rgn =mgr->GetPuMgr()->GetCGRegionMgr()->GetRegion(rgnIdx);
        AdjustBoxes4Rgn(rgn);AdjustBoxes4sideBlockage(mgr);
        int cellH = pu::GetDefaultcellHeight();
        SetMinRow(( h2+ cellH-1)/ cellH);
        SetMaxRow((h1+cellH-1)/cellH);
        SetAnalyzed(true);
    }
}

void gpIsoGroup::AdjustBoxes4Rgn(const puRegion *rgn){
    guRectList bndy, bndy2;

    GetRectList(&bndy);
    bndy2.setAnd(&bndy,rgn->GetPlacecore());

    int wl = bndy.GetBBox()->Getwidth();
    int hl = bndy.GetBBox()->GetHeight();
    int w2 = bndy2.GetBBox()->GetWidth();
    int h2 = bndy2.GetBBox()->GetHeight();
    if((w2< w1)||(h2< hl)|l isside){
        AdjustBoxes4size(w2,h2);
    }
    if( isside){
        AdjustBoxes4side(w2,h2);
    }
}

void gpIsoGroup::AdjustBoxes4side(int maxw, int maxH){
    bool adjust = false;
    if(axis){
        int minH =utceil(1.2*totArea / maxW);
        if( hl< minH){// one VER rect
            _wl=_w2 = maxW;
            _hl= _h2= minH;
        } else {
            adjust = true;
        }else{
            int minW=utceil(1.2 *totArea / maxH);
            if( w2 < minw){ // one HoR rect
                _w1=_w2 = minW;
                _hl=_h2 = maxH;
            } else {
                adjust = true;
            }
        }
    }
    if (adjust){
        int w3 = UTMIN(maxW,w2);
        int h3 = UTMIN(maxH, h1);
        int w4 = w3 - _w1;
        int h4 = h3 - _h2;
        if((w4>0)&&(h4 >0)){
            double w = _w2-_w3;
            double h = _h1- h3;
            double a3=w*_h2 + wl * h;
            double a4 = w4;
            a4 *= h4;
            double r4 = h4;
            r4 /= w4;
            double w5=sqrt((a4-a3)/ r4);
            double h5 =w5 * r4;
            _wl += utCeil(w5);
            _w2 = w3;
            _h1 = h3;
            _h2 += utCeil(h5);
        }
    }
}

void gpIsoGroup::AdjustBoxes4Blockage(gpIsoMgr *mgr){
    guRect boxl, box2;
    GetBlockageBox1(&box1);
    GetBlockageBox2(&box2);
    bool verBlocked = mgr->GetBlockageBin()->IsOverlapping(&box1)
    bool horBlocked =mgr->GetBlockageBin()->IsOverlapping(&box2);
    if (horBlocked !=verBlocked){
        double r= puParam::GetIsolationcellPlacementAspectRatio();
        double w, h;
        if (horBlocked){ // HoR blocked -> VER rectangle
            w=sqrt( totArea /r);h=W*r;
        }else {// VER blocked -> HoR rectangle
            h=sqrt( totArea /r);
            W=h*r;
        }
        _w1= _w2 = utCeil(w);
        _hl= _h2 = utCeil(h);
    }
}

void gpIsoGroup::AdjustBoxes4sideBlockage(gpIsoMgr *mgr){
    guRect boxl,box2;
    GetBlockageBox1(&box1);
    GetBlockageBox2(&box2);

    bool verBlocked = mgr->GetBlockageBin()->IsOverlapping(&box1);
    bool horBlocked = mgr->GetBlockageBin()->IsOverlapping(&box2);

    if(horBlocked != verBlocked){
        double r= puParam::GetIsolationcellPlacementAspectRatio();
        double w, h;
        if(horBlocked){// HoR blocked -> VER rectangle
            w=sqrt( totArea /r);
            h=w* r;
        }else { // VER blocked -> HoR rectangle
            h= sqrt( totArea /r);
            W =h*r;
        }
        if(axis){
            if(w<h){
                w1=w2= utceil(w);
                hl= h2 =utceil(h);
            }else{
                wl=w2 = utceil(h);
                hl= h2 =utceil(w);
            }
        }else{
            if(w<h){
                wl=w2=utceil(h);
                hl=h2=utceil(w);
            }else{
                w1=w2=utceil(w);
                h1=h2=utceil(h);
            }
        }
    }
}

void gpIsoGroup::PlaceIsocells(){
    if(!IsCornerGroup())
        return;
    GetIsocellList()->Sort(gpIsocell::compareDist);
    int h = pu::GetDefaultcellHeight();
    int pos[ maxRow];
    for(intr=0;r<maxRow;r++){
        pos[r]= 0;
    }
    int penalty =UTMAX(GetW2(),GetH1())<< 2;
    gpIsoCellIter iter;
    gpIsoCell *cell;
    for(iter.Begin(this);!iter.End(cell);iter++){
        int minCost=INT_MAX;
        int minRow=-1;
        int x,y;

        for(int r=0;r<maxRow;r++){
            int maxWidth =(r<minRow)? GetMaxWidth():GetMinWidth();
            x=GetX()+GetDirx()*pos[r];
            y=GetY()+GetDirY()*r*h;
            int cost=2*GetPoint()->GetXyDistance(x,y)+cell->GetLoc()->GetXyDistance(x,y);
            if(pos[r] >=maxWidth){
                cost +=penalty;
            }
            if(cost<mincost){
                minCost=cost;
                minRow=r;
            }
        }
        guPoint pt;
        x=GetX()+GetDirX()*(pos[minRow]+cell->GetPlWidth()/2);
        y=GetY()+GetDirY()*(minRow*h+h/2);
        pt.Set(x,y);

        pu::GetInst(cell->GetInst())->Shift(&pt);
        if(cell->GetHeight()== 1){
            pos[minRow]+= cell->GetWidth();
        } else {
            if(minRow +cell->GetHeight()>=maxRow){
                minRow=maxRow-cell->GetHeight();
            }
            for(int i=0;i<cell->GetHeight();i++){
                pos[minRow + i]+= cell->GetWidth();
            }
        }
    }
}

void gpIsoGroup::Dump() const {
    cout << "isoGrp("<< GetGrpIdx()<< ")\n";
    cout <<"rgn ="<< GetRgnIdx()<< "\n";
    cout <<"pt ="<< pu::Point2str(GetPoint()) << "\n";

    guRect bbox;
    GetBox1(&bbox);
    cout <<"bboxl="<< pu::BBox2Str(&bbox)<< "\n";
    GetBox2(&bbox);
    cout << "bbox2 ="<< pu::BBox2Str(&bbox)<< "\n";
}

void gpIsoGroup::Draw(int cross)const{
    guRect bbox;

    GetBoxl(&bbox);
    pu::DrawBBox(&bbox,cross);

    GetBox2(&bbox);
    pu::DrawBBox(&bbox,cross);
}

// VER boX
void gpIsoGroup::GetBoxl(guRect *bbox) const {
    bbox->SetPoint(GetPoint());
    int x2 = GetX()+ GetDirX()*GetW1();
    int y2 = GetY()+ GetDirY()*GetH1();
    bbox->Expand(x2,y2);
}

void gpIsoGroup::GetRectList(guRectList *bndy)const {
    guRect bbox, bbox2;
    GetBox1(&bbox);
    GetBox2(&bbox2);
    bndy->Reset();
    bndy->AddRect(&bbox);
    bndy->AddRect(&bbox2);
}

void gpIsoGroup::GetBlockageBoxl(guRect *bbox) const {
    bbox->SetPoint(GetPoint());
    int x2=GetX()-GetDirX()* GetWl();
    int y2 = GetY()+ GetDirY()*GetH1();
    bbox->Expand(x2,y2);
}










    












