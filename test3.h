/*************
Advanced Volume Rendering

///////////////

class VolumeRayCasterImp 
{
public:
    VolumeRayCaster *myInstance;

    VolumeRayCasterImp( VolumeRayCaster *instance )
        : myInstance(instance),  
        mySeisVol( NULL ),
        myAttrVol( NULL ), 
        myOctreeVol(NULL), 
        myVolOctreeConfig(NULL), 
        myVolRenderConfig(NULL), 
        myTransferFunctionInitialized(false),
        myExtraVolumeTransferFunctionInitialized(false),
        myTriangleMeshRenderPrg(NULL), 
        mySVOBuildProgress(0),
        myGridTexture3D(0),
        myTornadoView(NULL), 
        myWellLogManager(NULL), 
        myVisualizeHrzImage(false),
        myVisualizeSSAOImage(false), 
        myVisualizeSSAOBlurredImage(false)
        //,myLightSphere(NULL), 
        //myDrawObjectTechnique(NULL)
    {
        myVolOctreeConfig = new VolumeOctreeConfig; 
        myVolRenderConfig = new VolumeRenderConfig; 
        myVolumeRayCasterAlgo = new VolumeRayCasterAlgorithm(myVolOctreeConfig, myVolRenderConfig); 
        myVolRenderConfig->setInteractiveMode(AUTO);
        myVolumeRayCasterAlgo->resetRenderState(); 
        myVolumeRayCasterAlgo->setSSAOUseBlurredPass(true); 
        //myLightSphere = new vlGLSphere(40); 
        myClippingHrz[0] = myClippingHrz[1] = NULL; 
        myIsRGBAMode = false; 
    }

    ~VolumeRayCasterImp()
    {
        reset();
        disconnectSignal();
    }

    void reset();
    void connectSignal();
    void disconnectSignal();
   
    
    void setTornadoView(vmGLSmartView* torMainView); 
    void setVolumeSlateManager(VolumeSlateManager* vsm); 
    bool updateVolumeOctree( bool fromLDM); 
    bool updateVolumeOctreeRGBA(bool fromLDM); 
    
    void preRenderGLState(); 
    void postRenderGLState(); 
    bool preVoxelRender(); 
    void postVoxelRender( ); 
    void inVoxelRender(timeval t); 
    void renderEntryExit(); 
    
    void renderWellLogsPreZPass();
    void renderWellLogsShadingPass(); 

    //void renderTriangleMesh(); 

    bool initTransferFunc(Volume *seisVol, Volume* attrVol, Spectrum& seisVolSpect, Spectrum& attrVolSpect); 
    bool initExtraTransferFunc(Volume *extraVol, Spectrum& extraSpect); 
    
    vlSparseVolumeOctreeWrapper* createVolOctree( bool fromLDM  ); 
    vlSparseVolumeOctreeWrapper* createVolOctreeRGBA( bool fromLDM  ); 
    int getTornadoVolumeChannel(Volume* delVol) const; 
    //
    static void spectrum2array( Spectrum& spectrum, float*& array, float*& opacitySAT); 
    
    //object-space empty skipping 
    void computeGridTexture(Volume* torVolCh1, Volume* torVolCh2,  Vect3i gridDim, Vect3i voxelsPerCell ); // seismic mode: seismic, attribute mode: attribute, co-render: both
  
    void invalidateGridBuffer(); 
    void invalidateGridTexture(); 
    void invalidateTransferFunction(); 
    void invalidateExtraVolTransferFunction(); 
    
    void drawHrzTextureToScreen(int hid, Vect4i viewportSize); 
    void drawSSAOTextureToScreen(Vect4i viewportSize); 
    void drawSSAOBlurredTextureToScreen(Vect4i viewportSize); 
    
    void updateROI(); 
    
    void autoSetVRAMBudget(size_t octreeDim); 
    
    //void drawLights(); 
    void updateSceneGraph(); //svo third party
    void getSVOVolumeState(Volume*& activeVolSVO, Volume*& extraVolSVO) const; 
    
    
private:
    void drawTextureToScreen(const GLuint& texHandle, const Vect4i viewportSize); 
    //vlDrawObjectTechnique* getDrawObjectTechnique(); 
    
public:
    VolumeRayCasterAlgorithm* myVolumeRayCasterAlgo; 
    Volume *mySeisVol; 
    Volume *myAttrVol; 
    Spectrum mySeisVolSpec; //last time build svo
    Spectrum myAttrVolSpec; //last time build svo
    vString mySeisLDMPath; 
    vString myAttrLDMPath; 
    Spectrum myCurrSeisVolSpec; // for extra slice which not built svo
    Spectrum myCurrAttrVolSpec; // for extra slice which not built svo 
    
    vector<Volume*> myChannelVols; //for color blending
    vector<vString> myChannelLDMPaths; 
    bool myIsRGBAMode; 
    
    vlSparseVolumeOctreeRep* myOctreeVol; 
    VolumeOctreeConfig* myVolOctreeConfig;
    VolumeRenderConfig* myVolRenderConfig; 
    
    TaskStatus*  mySVOBuildProgress;
    bool myTransferFunctionInitialized; 
    bool myExtraVolumeTransferFunctionInitialized; 
    
    //for triangulated mesh rendering by using ray caster, obsolete
    TriangleMeshShaderViewGroup myTriangleMeshShaderViewG; 
    vlGLSLProgram* myTriangleMeshRenderPrg; 
    GLuint myTriangleMeshRenderPipeline; 
    GLint myTriangleMeshRenderMVPUniform; 
    GLint myLog2GrdMatrixUniform; 
    GLint myGeoSizeUniform;  
      
    //HORIZON CLIPP cache, 
    vgHorizon* myClippingHrz[2]; 
    bool myVisualizeHrzImage;
    GLuint myGridTexture3D; 
    
    //some cache to update listener
    map< SliceBase*, pair<SlateBase*, bool> > mySliceBaseListenerCache; 
    map< CurvedSlice*, pair<CurvedSlate*, bool> > myCurvedSliceListenerCache; 
    
    vmGLSmartView* myTornadoView; 
    WellLogManager* myWellLogManager; 
    VolumeSlateManager* myVSM; 
    
    bool myVisualizeSSAOImage; 
    bool myVisualizeSSAOBlurredImage; 
    
    //vlGLSphere* myLightSphere; 
    //vlDrawObjectTechnique* myDrawObjectTechnique; 
};

void VolumeRayCasterImp::connectSignal()
{
    vlAppModel& myModel = vlAppModel::getRef();
    VolumeRayCaster &volumeraycaster = *myInstance;
    Connect( myModel, myModel.NewCurrentHorizon, volumeraycaster, &VolumeRayCaster::onCurrentHorizonChange );      
    Connect (myModel, myModel.NewCurrentSeismic, volumeraycaster, &VolumeRayCaster::onNewCurrentSeismic);
    Connect (myModel, myModel.NewCurrentAttribute, volumeraycaster, &VolumeRayCaster::onNewCurrentAttribute);
    Connect( myModel, myModel.NewCurrentGeometry, volumeraycaster, &VolumeRayCaster::onNewCurrentReferenceGeometry);
    
    Connect( *myTornadoView, myTornadoView->GlLightParams,  volumeraycaster, &VolumeRayCaster::onUpdateGlLightProperty );
    Connect( myModel, myModel.PreUnloadHorizonGroup, volumeraycaster, &VolumeRayCaster::onHorizonGroupUnload  );
    
    if(this->myVSM)
    {
        Connect(*myVSM, myVSM ->BasicVis, volumeraycaster, &VolumeRayCaster::onNewBasicSliceVis);
    }
    
}

void VolumeRayCasterImp::disconnectSignal()
{
    vlAppModel& myModel = vlAppModel::getRef();
    
    VolumeRayCaster &volumeraycaster = *myInstance;
    Disconnect( myModel, myModel.NewCurrentHorizon, volumeraycaster, &VolumeRayCaster::onCurrentHorizonChange );    
    Disconnect (myModel, myModel.NewCurrentSeismic, volumeraycaster, &VolumeRayCaster::onNewCurrentSeismic);
    Disconnect (myModel, myModel.NewCurrentAttribute, volumeraycaster, &VolumeRayCaster::onNewCurrentAttribute);
    Disconnect( myModel, myModel.NewCurrentGeometry, volumeraycaster, &VolumeRayCaster::onNewCurrentReferenceGeometry);
    
    Disconnect( *myTornadoView, myTornadoView->GlLightParams,  volumeraycaster, &VolumeRayCaster::onUpdateGlLightProperty );
    Disconnect( myModel, myModel.PreUnloadHorizonGroup, volumeraycaster, &VolumeRayCaster::onHorizonGroupUnload  );
    
    if(this->myVSM)
    {
        Disconnect(*myVSM, myVSM ->BasicVis, volumeraycaster, &VolumeRayCaster::onNewBasicSliceVis);
    }
    
    for(map<SliceBase*, pair<SlateBase*, bool> >::iterator itr = mySliceBaseListenerCache.begin(); itr !=  mySliceBaseListenerCache.end(); ++itr)
    {
        SliceBase* channelSlice = itr->first; 
        Disconnect( *channelSlice, channelSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshBaseSliceChannel );    
    }
    for(map< CurvedSlice*, pair< CurvedSlate*, bool> >::iterator itr = myCurvedSliceListenerCache.begin(); itr !=  myCurvedSliceListenerCache.end(); ++itr)
    {
        CurvedSlice* channelSlice = itr->first; 
        Disconnect( *channelSlice, channelSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshCurvedSliceChannel );    
    }
}
// update scene graph:  for now it only means all slices, basic, traverse, fence, curved fence
// WARNING: rgba mode is not implemented for this. 
void VolumeRayCasterImp::updateSceneGraph() //svo third party
{
    Volume* activeVolSVO = NULL; 
    Volume* extraTorVol = NULL; 
    VolumeRayCaster &volumeraycaster = *myInstance;
    
    getSVOVolumeState(activeVolSVO, extraTorVol);

    bool isShowSeis = myVSM->getAllSeismicSlicesVis();
    bool isShowAttr = myVSM->getAllAttributeSlicesVis();
    //basic slice
    char sliceName[128]; 
    for(int i = 0; i < 3; i++)
    {
        int baseslateIdx = i;
        SlateBase* curBSlate= myVSM->getBasicSlate(baseslateIdx); 
        if(extraTorVol)
        {
            int volIdx = curBSlate->getVolumeIndex(*(extraTorVol));
            SliceBase* channelSlice = curBSlate->getSlice(volIdx); 
            ColorSlice* torSlice = dynamic_cast<ColorSlice*>(channelSlice); 
            if(torSlice && !torSlice->isInvalid())
            {          
                Vect4d bgn_vol, end_vol;
                int axis_h; 
                torSlice->getDefin( axis_h, bgn_vol, end_vol ); //survey coord, 
                Vect2i slc_size = extraTorVol->getSliceSize( bgn_vol, end_vol );
                Matrixd l2g_vol;
                //cube
                Vect4d cbgn_vol, cend_vol;
                l2g_vol = extraTorVol->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID);      
                cbgn_vol = l2g_vol * bgn_vol;
                cend_vol = l2g_vol * end_vol;

                float* tmp = new float[slc_size[0] * slc_size[1]];
                extraTorVol->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                assert(tmp); 

//              myVolumeRayCasterAlgo->updateBasicSliceNode(i, 2, cbgn_vol, cend_vol, tmp, slc_size, 
//                        curBSlate->isVisible(),  isShowSeis, isShowAttr ); 
                vector<vlSceneGraphNode*> nodes;
                sprintf(sliceName, "Ortho_%d", i);
                vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
                if(nodes.size() == 0 )
                {            
                    vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
                    vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode);
                    OrthoSliceRenderable* orthoslice = new OrthoSliceRenderable(); 
                    newNode->addComponent(orthoslice);         
                }
         
                if(this->mySliceBaseListenerCache.count(channelSlice) == 0)
                { 
                    Connect( *channelSlice, channelSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshBaseSliceChannel );    
                    mySliceBaseListenerCache.insert(make_pair(channelSlice, make_pair(curBSlate, true))); 
                }
            }             
        }

        if(activeVolSVO)
        {
            int volIdx = curBSlate->getVolumeIndex(*(activeVolSVO));
            SliceBase* channelSlice = curBSlate->getSlice(volIdx); 
            ColorSlice* torSlice = dynamic_cast<ColorSlice*>(channelSlice); 
            if( torSlice && !torSlice ->isInvalid())
            {
                Vect4d bgn_vol, end_vol;
                int axis_h; 
                torSlice->getDefin( axis_h, bgn_vol, end_vol ); //survey coord, 
                Vect2i slc_size = activeVolSVO->getSliceSize( bgn_vol, end_vol );
                Matrixd l2g_vol;
                //cube
                Vect4d cbgn_vol, cend_vol;
                l2g_vol = activeVolSVO->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID);      
                cbgn_vol = l2g_vol * bgn_vol;
                cend_vol = l2g_vol * end_vol;
                float* tmp = new float[slc_size[0] * slc_size[1]];
                activeVolSVO->makeSliceN2P( bgn_vol, end_vol, 32, tmp);  
                assert(tmp); 
//                myVolumeRayCasterAlgo->updateBasicSliceNode(i, 0, cbgn_vol, cend_vol, tmp, slc_size, 
//                        curBSlate->isVisible(), isShowSeis, isShowAttr ); 
                vector<vlSceneGraphNode*> nodes;
                sprintf(sliceName, "Ortho_%d", i);
                vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
                if(nodes.size() == 0 )
                {            
                    vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
                    vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode);
                    OrthoSliceRenderable* orthoslice = new OrthoSliceRenderable(); 
                    newNode->addComponent(orthoslice);         
                }
                if(this->mySliceBaseListenerCache.count(channelSlice) == 0)
                { 
                   Connect( *channelSlice, channelSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshBaseSliceChannel );    
                    mySliceBaseListenerCache.insert(make_pair(channelSlice, make_pair(curBSlate,true))); 
                }
                if( myOctreeVol->getNumVolumes() > 1)
                {       
                    float* tmp = new float[slc_size[0] * slc_size[1]];
                    if( activeVolSVO == mySeisVol ) 
                    {
                        myAttrVol->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                    }
                    else
                    {
                        mySeisVol->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                    }
                    assert(tmp); 
//                    myVolumeRayCasterAlgo->updateBasicSliceNode(i, 1, cbgn_vol, cend_vol, tmp, slc_size, 
//                        curBSlate->isVisible(), isShowSeis, isShowAttr );  
                }
            }
        }
        myVolumeRayCasterAlgo->attachBasicSliceNodeExtraVolChannel(i, extraTorVol!=NULL); 
    }

    ////traverse slice 
    for(int i = 0; i < myVSM->getNumSlates(); i++)
    {
        SlateBase* curTSlate= myVSM->getSlate(i); 
        if(extraTorVol)
        {
            int volIdx = curTSlate->getVolumeIndex(*(extraTorVol));
            SliceBase* channelSlice = curTSlate->getSlice(volIdx); 
            ColorSlice* torSlice = dynamic_cast<ColorSlice*>(channelSlice); 
            if(torSlice && !torSlice->isInvalid())
            {          
                Vect4d bgn_vol, end_vol;
                int axis_h; 
                torSlice->getDefin( axis_h, bgn_vol, end_vol ); //survey coord, 
                Vect2i slc_size = extraTorVol->getSliceSize( bgn_vol, end_vol );
                Matrixd l2g_vol;
                //cube
                Vect4d cbgn_vol, cend_vol;
                l2g_vol = extraTorVol->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID);      
                cbgn_vol = l2g_vol * bgn_vol;
                cend_vol = l2g_vol * end_vol;

                float* tmp = new float[slc_size[0] * slc_size[1]];
                extraTorVol->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                assert(tmp); 
//                myVolumeRayCasterAlgo->updateTraverseSliceNode(i, 2, cbgn_vol, cend_vol, tmp, slc_size, 
//                        curTSlate->isVisible(), isShowSeis, isShowAttr ); 
                
                vector<vlSceneGraphNode*> nodes;
                sprintf(sliceName, "Traverse_%d", i);
                vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
                if(nodes.size() == 0 )
                {            
                    vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
                    vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode); 
                    OrthoSliceRenderable* orthoslice = new OrthoSliceRenderable(); 
                    newNode->addComponent(orthoslice);     
                }
                if(this->mySliceBaseListenerCache.count(channelSlice) == 0)
                { 
                    Connect( *channelSlice, channelSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshBaseSliceChannel );    
                    mySliceBaseListenerCache.insert(make_pair(channelSlice, make_pair(curTSlate,true))); 
                }
            }             
        }

        if(activeVolSVO)
        {
            int volIdx = curTSlate->getVolumeIndex(*(activeVolSVO));
            SliceBase* channelSlice = curTSlate->getSlice(volIdx); 
            ColorSlice* torSlice = dynamic_cast<ColorSlice*>(channelSlice); 
            if( torSlice && !torSlice ->isInvalid() )
            {
                Vect4d bgn_vol, end_vol;
                int axis_h; 
                torSlice->getDefin( axis_h, bgn_vol, end_vol ); //survey coord, 
                Vect2i slc_size = activeVolSVO->getSliceSize( bgn_vol, end_vol );
                Matrixd l2g_vol;
                //cube
                Vect4d cbgn_vol, cend_vol;
                l2g_vol = activeVolSVO->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID);      
                cbgn_vol = l2g_vol * bgn_vol;
                cend_vol = l2g_vol * end_vol;

                float* tmp = new float[slc_size[0] * slc_size[1]];
                activeVolSVO->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                assert(tmp); 
//                myVolumeRayCasterAlgo->updateTraverseSliceNode(i, 0, cbgn_vol, cend_vol, tmp, slc_size, 
//                        curTSlate->isVisible(), isShowSeis, isShowAttr ); 
                vector<vlSceneGraphNode*> nodes;
                sprintf(sliceName, "Traverse_%d", i);
                vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
                if(nodes.size() == 0 )
                {            
                    vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
                    vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode); 
                    OrthoSliceRenderable* orthoslice = new OrthoSliceRenderable(); 
                    newNode->addComponent(orthoslice);     
                }
                
                if(this->mySliceBaseListenerCache.count(channelSlice) == 0)
                { 
                    Connect( *channelSlice, channelSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshBaseSliceChannel );    
                    mySliceBaseListenerCache.insert(make_pair(channelSlice, make_pair(curTSlate,true))); 
                }
                if( myOctreeVol->getNumVolumes() > 1)
                {       
                    float* tmp = new float[slc_size[0] * slc_size[1]];
                    if( activeVolSVO == mySeisVol ) 
                    {
                        myAttrVol ->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                    }
                    else
                    {
                        mySeisVol ->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                    }
                    assert(tmp); 
//                    myVolumeRayCasterAlgo->updateTraverseSliceNode(i, 1, cbgn_vol, cend_vol, tmp, slc_size, 
//                        curTSlate->isVisible(), isShowSeis, isShowAttr ); 
                }
            }
        }
        myVolumeRayCasterAlgo->attachTraverseSliceNodeExtraVolChannel(i, extraTorVol!=NULL); 
    }

    //FENCE
    for(int fid = 0; fid < myVSM->getNumFenceSet(); fid++)
    {
        vector<SlateBase*>& fenceInSet = myVSM->getFenceSetByIdx(fid);
        
        for(int i = 0; i < fenceInSet.size(); i++)
        {
            SlateBase* curFSlate= fenceInSet[i]; 

            if(extraTorVol)
            {
                int volIdx = curFSlate->getVolumeIndex(*(extraTorVol));
                SliceBase* channelSlice = curFSlate->getSlice(volIdx); 
                ColorSlice* torSlice = dynamic_cast<ColorSlice*>(channelSlice); 

                if(torSlice && !torSlice->isInvalid())
                {          
                    Vect4d bgn_vol, end_vol;
                    int axis_h; 
                    torSlice->getDefin( axis_h, bgn_vol, end_vol ); //survey coord, 
                    Vect2i slc_size = extraTorVol->getSliceSize( bgn_vol, end_vol );
                    Matrixd l2g_vol;
                    //cube
                    Vect4d cbgn_vol, cend_vol;
                    l2g_vol = extraTorVol->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID);      
                    cbgn_vol = l2g_vol * bgn_vol;
                    cend_vol = l2g_vol * end_vol;

                    float* tmp = new float[slc_size[0] * slc_size[1]];
                    extraTorVol->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                    assert(tmp);
    //                myVolumeRayCasterAlgo->updateFenceSliceNode(i, 2, cbgn_vol, cend_vol, tmp, slc_size, 
    //                       curFSlate->isVisible(), isShowSeis, isShowAttr ); 
                    vector<vlSceneGraphNode*> nodes;
                    sprintf(sliceName, "Fence_%d_%d", fid, i);
                    vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
                    if(nodes.size() == 0 )
                    {            
                        vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
                        vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode); 
                        OrthoSliceRenderable* orthoslice = new OrthoSliceRenderable(); 
                        newNode->addComponent(orthoslice);  
                    }
                    if(this->mySliceBaseListenerCache.count(channelSlice) == 0)
                    { 
                        Connect( *channelSlice, channelSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshBaseSliceChannel );    
                        mySliceBaseListenerCache.insert(make_pair(channelSlice, make_pair(curFSlate,false))); 
                    }
                }             
            }

            if(activeVolSVO)
            {
                int volIdx = curFSlate->getVolumeIndex(*(activeVolSVO));
                SliceBase* channelSlice = curFSlate->getSlice(volIdx); 
                ColorSlice* torSlice = dynamic_cast<ColorSlice*>(channelSlice); 
                if( torSlice && !torSlice ->isInvalid() )
                {
                    Vect4d bgn_vol, end_vol;
                    int axis_h; 
                    torSlice->getDefin( axis_h, bgn_vol, end_vol ); //survey coord, 
                    Vect2i slc_size = activeVolSVO->getSliceSize( bgn_vol, end_vol );
                    Matrixd l2g_vol;
                    //cube
                    Vect4d cbgn_vol, cend_vol;
                    l2g_vol = activeVolSVO->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID);      
                    cbgn_vol = l2g_vol * bgn_vol;
                    cend_vol = l2g_vol * end_vol;

                    float* tmp = new float[slc_size[0] * slc_size[1]];
                    activeVolSVO->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                    assert(tmp); 
    //                myVolumeRayCasterAlgo->updateFenceSliceNode(i, 0, cbgn_vol, cend_vol, tmp, slc_size, 
    //                       curFSlate->isVisible(), isShowSeis, isShowAttr );
                    vector<vlSceneGraphNode*> nodes;
                    sprintf(sliceName, "Fence_%d_%d", fid, i);
                    vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
                    if(nodes.size() == 0 )
                    {            
                        vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
                        vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode); 
                        OrthoSliceRenderable* orthoslice = new OrthoSliceRenderable(); 
                        newNode->addComponent(orthoslice);  
                    }
                    if(this->mySliceBaseListenerCache.count(channelSlice) == 0)
                    { 
                        Connect( *channelSlice, channelSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshBaseSliceChannel );    
                        mySliceBaseListenerCache.insert(make_pair(channelSlice, make_pair(curFSlate,false))); 
                    }
                    if( myOctreeVol->getNumVolumes() > 1)
                    {       

                        float* tmp = new float[slc_size[0] * slc_size[1]];
                        if( activeVolSVO == mySeisVol ) 
                        {
                            myAttrVol ->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                        }
                        else
                        {
                            mySeisVol ->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                        }
                        assert(tmp); 

    ////                    myVolumeRayCasterAlgo->updateFenceSliceNode(i, 1, cbgn_vol, cend_vol, tmp, slc_size, 
    ////                       curFSlate->isVisible(), isShowSeis, isShowAttr );
                    }
                }
            }
            myVolumeRayCasterAlgo->attachFenceSliceNodeExtraVolChannel(fid, i, extraTorVol!=NULL); 
        }
    }
    //curved fence 
    for(int i = 0; i < myVSM->getNumCurvedSlates(); i++)
    {
        CurvedSlate* cs = myVSM->getCurvedSlate(i);
        if(extraTorVol)
        {
            int volIdx = cs->getVolumeIndex(*(extraTorVol));         
            CurvedSlice* curvedSlice = cs->getCurvedSlice(volIdx); 
            if(curvedSlice )
            {   
//                vector<Vect4d> pts = curvedSlice->getCurveDrawPts();
//                vector<float*> traces = curvedSlice->getCurveDrawTraces();
//                myVolumeRayCasterAlgo->updateCurvedFenceSliceNode( i, 2, pts, traces, 
//                        curvedSlice->getCurveZOrig(), 
//                        curvedSlice->getCurveZStep(), 
//                        curvedSlice->getCurveZNumSamples(), 
//                        extraTorVol->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID), 
//                        curvedSlice->isVisible(), isShowSeis, isShowAttr ); 
                vector<vlSceneGraphNode*> nodes;
                sprintf(sliceName, "CurvedFence_%d", i);
                vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
                if(nodes.size() == 0 )
                {            
                    vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
                    vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode); 
                    CurvedSliceRenderable* curvedSliceRend = new CurvedSliceRenderable(); 
                    newNode->addComponent(curvedSliceRend);  
                }
                
                if(this->myCurvedSliceListenerCache.count(curvedSlice) == 0)
                { 
                    Connect( *curvedSlice, curvedSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshCurvedSliceChannel );    
                    myCurvedSliceListenerCache.insert( make_pair( curvedSlice, make_pair(cs, false) ) ); 
                }
            }             
        }
        if(activeVolSVO)
        {
            int volIdx = cs->getVolumeIndex(*(activeVolSVO));
            CurvedSlice* curvedSlice = cs->getCurvedSlice(volIdx); 
            if(curvedSlice)
            {
//                vector<Vect4d> pts = curvedSlice->getCurveDrawPts();
//                vector<float*> traces = curvedSlice->getCurveDrawTraces();
//                myVolumeRayCasterAlgo->updateCurvedFenceSliceNode(i, 0, pts, traces, 
//                        curvedSlice->getCurveZOrig(), 
//                        curvedSlice->getCurveZStep(), 
//                        curvedSlice->getCurveZNumSamples(), 
//                        activeVolSVO->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID), 
//                         curvedSlice->isVisible(), isShowSeis, isShowAttr ); 
                vector<vlSceneGraphNode*> nodes;
                sprintf(sliceName, "CurvedFence_%d", i);
                vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
                if(nodes.size() == 0 )
                {            
                    vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
                    vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode); 
                    CurvedSliceRenderable* orthoslice = new CurvedSliceRenderable(); 
                    newNode->addComponent(orthoslice);  
                }
                if(this->myCurvedSliceListenerCache.count(curvedSlice) == 0)
                { 
                    Connect( *curvedSlice, curvedSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshCurvedSliceChannel );    
                    myCurvedSliceListenerCache.insert(make_pair(curvedSlice, make_pair(cs,false))); 
                }
            } 
            if( myOctreeVol->getNumVolumes() > 1)
            {
                int volIdx; 
                Volume* coRenderVolume; 
                if( activeVolSVO == mySeisVol ) 
                {
                    coRenderVolume = myAttrVol; 
                }
                else
                {
                    coRenderVolume = mySeisVol;
                }
                volIdx = cs -> getVolumeIndex(*coRenderVolume);
                CurvedSlice* curvedSlice = cs->getCurvedSlice(volIdx); 
                if(curvedSlice)
                {
//                    vector<Vect4d> pts = curvedSlice->getCurveDrawPts();
//                    vector<float*> traces = curvedSlice->getCurveDrawTraces();
//                    myVolumeRayCasterAlgo->updateCurvedFenceSliceNode(i, 1, pts, traces, 
//                            curvedSlice->getCurveZOrig(), 
//                            curvedSlice->getCurveZStep(), 
//                            curvedSlice->getCurveZNumSamples(), 
//                            coRenderVolume->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID), 
//                            curvedSlice->isVisible(), isShowSeis, isShowAttr );     
                    if(this->myCurvedSliceListenerCache.count(curvedSlice) == 0)
                    { 
                        Connect( *curvedSlice, curvedSlice->Refresh, volumeraycaster, &VolumeRayCaster::onRefreshCurvedSliceChannel );    
                        myCurvedSliceListenerCache.insert(make_pair(curvedSlice, make_pair(cs,false))); 
                    }
                } 
            }
        }
        myVolumeRayCasterAlgo->attachCurvedFenceNodeExtraVolChannel(i, extraTorVol!=NULL); 
    }
}

//this is poorly designed. Main idea, is to differentiate a tornado volume and a svo built volume. 
//extraVolTor is a tornado volume which not considered to have volume rendering feature, but need to have some rendering impact on volume rendering pipeline. 
//ex: depth buffer..
void VolumeRayCasterImp::getSVOVolumeState(Volume*& activeVolSVO, Volume*& extraVolTor) const
{
    activeVolSVO = NULL; 
    extraVolTor = NULL; 
    if(mySeisVol)
        activeVolSVO = mySeisVol; 
    else
        activeVolSVO = myAttrVol; 
    bool needExtraTorVol = ( (mySeisVol && !myAttrVol) || (!mySeisVol && myAttrVol) );
    if(needExtraTorVol)
    {
        if(activeVolSVO == mySeisVol)
            extraVolTor = vlAppModel::getRef().getCurAttribute();
        else
            extraVolTor = vlAppModel::getRef().getCurSeismic();
    }  
}

void VolumeRayCasterImp::setTornadoView(vmGLSmartView* torMainView)
{
    this->myTornadoView = torMainView; 
}

void VolumeRayCasterImp::setVolumeSlateManager(VolumeSlateManager* vsm)
{
    this->myVSM = vsm; 
}

bool VolumeRayCasterImp::updateVolumeOctree( bool fromLDM )
{
    if( myOctreeVol )
    {
        delete myOctreeVol; 
        myOctreeVol = 0; 
        myVolumeRayCasterAlgo->setCurrentOctreeVol(NULL); 
    }
    
    if( !myOctreeVol )
    {
        myOctreeVol = createVolOctree(fromLDM);   
    }
    
    if( myOctreeVol )
    {
//       fprintf( stderr, "%s", myOctreeVol->getClassDescription().c_str() ); 
       myVolumeRayCasterAlgo->setCurrentOctreeVol( myOctreeVol->getSparseVolumeOctree());
       return true; 
    }
    else
    {
        fprintf(stderr, "failed to create SVO \n"); 
        return false; 
    }
}

bool VolumeRayCasterImp::updateVolumeOctreeRGBA(bool fromLDM)
{
    if( myOctreeVol )
    {
        delete myOctreeVol; 
        myOctreeVol = 0; 
        myVolumeRayCasterAlgo->setCurrentOctreeVol(NULL); 
    }
    
    if( !myOctreeVol )
    {
        myOctreeVol = createVolOctreeRGBA(fromLDM);   
    }
    
    if( myOctreeVol )
    {
//       fprintf( stderr, "%s", myOctreeVol->getClassDescription().c_str() ); 
       myVolumeRayCasterAlgo->setCurrentOctreeVol( myOctreeVol->getSparseVolumeOctree());
       return true; 
    }
    else
    {
        fprintf(stderr, "failed to create SVO \n"); 
        return false; 
    }
}

vlSparseVolumeOctreeWrapper* VolumeRayCasterImp::createVolOctree( bool fromLDM  ) 
{
    if( !mySeisVol && !myAttrVol)
    {
        fprintf(stderr, "No tornado volume to construct the svo representation\n");
        return NULL; 
    }
       
    float homogeneityThreshold = myVolOctreeConfig->getHomogeneityThreshold();    
    vlSparseVolumeOctreeWrapper* octree = NULL; 
   
    int brickDim =  (int)myVolOctreeConfig->getBrickDim(); 
   
    
    if( !OctreeUtils::isPowerOfTwo(brickDim) )
    {
        fprintf(stderr, "brick must be power of two per dimension\n"); 
        return NULL;        
    }

    std::vector< VolumeDataProvider*> torVolumes; 
    if(mySeisVol)
        torVolumes.push_back(mySeisVol); 
    if(myAttrVol)
        torVolumes.push_back(myAttrVol); 
   
    size_t mode; 
    if(mySeisVol)
    {
        if( myAttrVol )
           mode = OctreeUtils::VOLUMERENDER_CORENDER; 
        else
           mode = OctreeUtils::VOLUMERENDER_SEIS; 
    }
    else
    {
        if( myAttrVol )
            mode = OctreeUtils::VOLUMERENDER_ATTR; 
        else
            mode = OctreeUtils::VOLUMERENDER_UNKNOWN; 
    }
   
    if(mode == OctreeUtils::VOLUMERENDER_UNKNOWN)
    {
        fprintf(stderr, "Unknown SVO Mode\n"); 
        return NULL; 
    }
    
    const Vect3i volDim = (torVolumes[0] != NULL ? Vect3i( torVolumes[0]->getVoxelSize()[0],torVolumes[0]->getVoxelSize()[1],
            torVolumes[0]->getVoxelSize()[2]) : 
                   Vect3i( torVolumes[1]->getVoxelSize()[0],torVolumes[1]->getVoxelSize()[1],torVolumes[1]->getVoxelSize()[2])  );  \

    
    vString data_dir; 
    vString dir;
    vString name; 
    switch(mode)
    {
        case OctreeUtils::VOLUMERENDER_SEIS:
            data_dir = this->mySeisLDMPath + "BrickCache"; 
            break; 
        case OctreeUtils::VOLUMERENDER_ATTR:
            data_dir = this->myAttrLDMPath + "BrickCache"; 
            break; 
        case OctreeUtils::VOLUMERENDER_CORENDER:
            data_dir = this->mySeisLDMPath + "BrickCache"; 
            break; 
        default: 
            ; 
    }
    data_dir.splitAtLastToken(dir, name, '/'); 
    data_dir = dir + "BrickCache"; 
    if(!vUtl::directoryExists(data_dir.c_str() ) )
    {
        vUtl::mkdirRecursive (data_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    //OctreeUtils::getAbsolutePath("BrickCache"); 
    //fprintf(stderr, "BrickCache: %s\n", data_dir.c_str());
    
    if(!fromLDM )
    {
        
        octree = new vlSparseVolumeOctreeWrapper( torVolumes, (int)brickDim, volDim,  myVolOctreeConfig->isUsingSparseOptimization() ,
                homogeneityThreshold, myVolOctreeConfig->isUsingDiskSwap(), myVolOctreeConfig->getSingleBufferMemorySize(),
                myVolOctreeConfig->getCPURamLimit(), string( data_dir.c_str() ), string(OctreeUtils::getRandomSwapBufferName().c_str()),
                myVolOctreeConfig -> getNumThreads(), false,
               (size_t)mode, this->mySVOBuildProgress,  myVolOctreeConfig->isUsingIntertwineLayout()); 
        
        if( octree && !octree ->isValidSVO() )
        {
            delete octree; 
            octree = 0; 
        }
    }
    else
    {
        vector<string> ldmFiles; 
        if(mySeisVol)
            ldmFiles.push_back(string( mySeisLDMPath.c_str() ) );
        if(myAttrVol)
            ldmFiles.push_back(string( myAttrLDMPath.c_str() ) );

        octree = new  vlSparseVolumeOctreeWrapper(torVolumes,  ldmFiles, homogeneityThreshold, myVolOctreeConfig->isUsingDiskSwap(), myVolOctreeConfig->getSingleBufferMemorySize(),
                myVolOctreeConfig->getCPURamLimit(), string( data_dir.c_str() ), string(OctreeUtils::getRandomSwapBufferName().c_str()), myVolOctreeConfig->getNumThreads(), 
                (size_t)mode, this->mySVOBuildProgress , 
                myVolOctreeConfig->isUsingIntertwineLayout() ); 
        
        if( octree && !octree ->isValidSVO() )
        {
            delete octree; 
            octree = 0; 
        }
    }   
    
    return octree; 
}

vlSparseVolumeOctreeWrapper* VolumeRayCasterImp::createVolOctreeRGBA( bool fromLDM  )
{
    if( this->myChannelVols.size() == 0 )
    {
        fprintf(stderr, "No tornado channel volume to construct the svo\n");
        return NULL; 
    }
    if (this->myChannelVols.size() > 4)
    {
        fprintf(stderr, "Exceed max 4 tornado channel volume to construct the svo\n");
        return NULL; 
    }
       
    float homogeneityThreshold = myVolOctreeConfig->getHomogeneityThreshold();    
    vlSparseVolumeOctreeWrapper* octree = NULL; 
   
    int brickDim =  (int)myVolOctreeConfig->getBrickDim(); 
    
    if( !OctreeUtils::isPowerOfTwo(brickDim) )
    {
        fprintf(stderr, "brick must be power of two per dimension\n"); 
        return NULL;        
    }

    std::vector< VolumeDataProvider*> torVolumes; 
    for(int i = 0; i < myChannelVols.size(); i++)
    {
        Volume* tmp = myChannelVols.at(i); 
        torVolumes.push_back(tmp); 
    } 
   
    size_t mode = OctreeUtils::VOLUMERENDER_RGBA; 

    const Vect3i volDim = Vect3i( myChannelVols[0]->getVoxelSize()[0], myChannelVols[0]->getVoxelSize()[1], myChannelVols[0]->getVoxelSize()[2]); 
  
    vString data_dir; 
    vString dir;
    vString name; 
    data_dir = myChannelLDMPaths[0] + "BrickCache"; 
    data_dir.splitAtLastToken(dir, name, '/'); 
    data_dir = dir + "BrickCache"; 
    if(!vUtl::directoryExists(data_dir.c_str() ) )
    {
        vUtl::mkdirRecursive (data_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
    if(!fromLDM )
    {
        
        octree = new vlSparseVolumeOctreeWrapper( torVolumes, (int)brickDim, volDim,  myVolOctreeConfig->isUsingSparseOptimization() ,
                homogeneityThreshold, myVolOctreeConfig->isUsingDiskSwap(), myVolOctreeConfig->getSingleBufferMemorySize(),
                myVolOctreeConfig->getCPURamLimit(), string( data_dir.c_str() ), string(OctreeUtils::getRandomSwapBufferName().c_str()),
                myVolOctreeConfig -> getNumThreads(), false,
               (size_t)mode, this->mySVOBuildProgress,  myVolOctreeConfig->isUsingIntertwineLayout()); 
        
        if( octree && !octree ->isValidSVO() )
        {
            delete octree; 
            octree = 0; 
        }
    }
    else
    {
        vector<string> ldmFiles; 
        for(int i = 0; i < myChannelLDMPaths.size(); i++)
        {
            ldmFiles.push_back(string(myChannelLDMPaths[i].c_str())); 
        } 
        
        octree = new  vlSparseVolumeOctreeWrapper(torVolumes,  ldmFiles, homogeneityThreshold, myVolOctreeConfig->isUsingDiskSwap(), myVolOctreeConfig->getSingleBufferMemorySize(),
                myVolOctreeConfig->getCPURamLimit(), string( data_dir.c_str() ), string(OctreeUtils::getRandomSwapBufferName().c_str()), myVolOctreeConfig->getNumThreads(), 
                (size_t)mode, this->mySVOBuildProgress , 
                myVolOctreeConfig->isUsingIntertwineLayout() ); 
        
        if( octree && !octree ->isValidSVO() )
        {
            delete octree; 
            octree = 0; 
        }
    }   
    
    return octree; 
}

void VolumeRayCasterImp::reset()
{ 
    mySeisVol = myAttrVol = NULL;
    
    //empty space skipping
    vlGLExt::glDeleteTextures(1, &myGridTexture3D ); 
    
    if(myVolumeRayCasterAlgo)
        delete myVolumeRayCasterAlgo; 
    
    if(myVolOctreeConfig)
        delete myVolOctreeConfig; 
    
    if(myVolRenderConfig )
        delete myVolRenderConfig;   
    
//    if(this->myLightSphere)
//        delete myLightSphere; 
    
    myGridTexture3D = 0; 
    myVolOctreeConfig = 0 ;
    myVolumeRayCasterAlgo = 0; 
    myVolRenderConfig = 0; 
//    myLightSphere = 0; 
}

void VolumeRayCasterImp::preRenderGLState()
{
    //store the gl state before entering volume rendering loop
    glPixelTransferi( GL_MAP_COLOR, 0 ); // turn off in case this was on
//    GLboolean depth, cull, blend; 
//    GLint cullface; 
//    GLfloat clearColor[4]; 

//    glGetBooleanv(GL_DEPTH_TEST, &depth); 
//    glGetBooleanv( GL_CULL_FACE, &cull); 
//    glGetIntegerv( GL_CULL_FACE_MODE, &cullface);
//    glGetFloatv( GL_COLOR_CLEAR_VALUE, &clearColor[0] ); 
//    glGetBooleanv(GL_BLEND, &blend); 

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void VolumeRayCasterImp::postRenderGLState()
{
    //reset to default gl state after volume rendering pipeline, no break the tornado rendering
    vlGLExt::glBindFramebufferEXT( GL_FRAMEBUFFER, 0 ); 
            
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();
    glPixelTransferi( GL_MAP_COLOR, 1 ); 

    glDisable( GL_CULL_FACE ); 
    glEnable( GL_DEPTH_TEST); 
    glCullFace(GL_BACK);
    glDisable( GL_BLEND); 
    //glActiveTexture(GL_TEXTURE0); 
}

//delayed intialization for transferfunction 
bool VolumeRayCasterImp::preVoxelRender()
{  
    if(!this->myVolumeRayCasterAlgo->preRenderLoop())
    {
        return false; 
    }
    
    if(!this->myIsRGBAMode)
    {
         if(!myTransferFunctionInitialized)
        {
            if(!initTransferFunc(mySeisVol, myAttrVol, mySeisVolSpec, myAttrVolSpec))
                return false; 

            myTransferFunctionInitialized = true; 
        }
        
        
        Volume* activeVolume = mySeisVol ? mySeisVol : myAttrVol; 
        Volume* extraVolume = NULL;
        Spectrum extraVolSpect; 
        if( (mySeisVol && myAttrVol) == 0 )
        {
            if(activeVolume == mySeisVol)
            {
                extraVolume = vlAppModel::getRef().getCurAttribute();
            }
            else
            {
                extraVolume = vlAppModel::getRef().getCurSeismic();
            }
        }
        extraVolSpect = ( extraVolume == vlAppModel::getRef().getCurAttribute() ? myCurrAttrVolSpec : myCurrSeisVolSpec );

        if(extraVolume && !myExtraVolumeTransferFunctionInitialized)
        {
            if( !this->initExtraTransferFunc(extraVolume, extraVolSpect) )
                return false ; 

            myExtraVolumeTransferFunctionInitialized = true; 
        }
    }
    
    return true; 
}

//void VolumeRayCasterImp::renderTriangleMesh()
//{
////    glPixelTransferi( GL_MAP_COLOR, 0 ); // turn off in case this was on
////    GLboolean depth, cull, blend; 
////    GLint cullface; 
////    GLfloat clearColor[4]; 
////
////    glGetBooleanv(GL_DEPTH_TEST, &depth); 
////    glGetBooleanv( GL_CULL_FACE, &cull); 
////    glGetIntegerv( GL_CULL_FACE_MODE, &cullface);
////    glGetFloatv( GL_COLOR_CLEAR_VALUE, &clearColor[0] ); 
////    glGetBooleanv(GL_BLEND, &blend); 
////
////    glEnable(GL_DEPTH_TEST);
////    glEnable(GL_CULL_FACE);
////    glCullFace(GL_BACK);
////
////    glMatrixMode(GL_MODELVIEW);
////    glPushMatrix();
////    glLoadIdentity();
////
////    //
////    myTriangleMeshRenderPrg -> use(); 
////    vlGLExt::glUniformMatrix4fv(myTriangleMeshRenderMVPUniform, 1, 0, (float*)&myMVPMatrix.mat);
////    {
////        BaseGeometry* geom = vlAppModel::getRef().getCurGeometry();
////        Matrixd log2grdMat = geom->calcMatrix( BaseGeometry::LOGICAL, BaseGeometry::GRID ); 
////        Vect4i geoSize = geom ->getSize();
////        Matrixf log2grdMatf; 
////        for( int i = 0; i < 16; i++)
////            log2grdMatf[i] = (float)log2grdMat[i]; 
////        vlGLExt::glUniformMatrix4fv(myLog2GrdMatrixUniform, 1, 0, (float*)&log2grdMatf.mat);
////        vlGLExt::glUniform4i(myGeoSizeUniform, geoSize[0], geoSize[1], geoSize[2], geoSize[3]);    
////    }
////    myTriangleMeshRenderPrg -> release(); 
////
////    vlGLExt::glBindFramebufferEXT( GL_FRAMEBUFFER, myEntryExitBound.fboHandle[0] ); 
////    glClearColor(0,0,0,0); 
////    glClearDepth ( 1.0 );
////    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
////    glDepthFunc(GL_LEQUAL);
////    vlGLExt::glBindProgramPipeline(myTriangleMeshRenderPipeline);      
////    this->myTriangleMeshShaderViewG.render();
////    vlGLExt::glBindProgramPipeline(0);        
////
////    inVoxelRender(true); //actually render
////        
////    glEnable(GL_CULL_FACE);
////    glCullFace(GL_BACK);
////    glEnable(GL_BLEND); 
////    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
////    glEnable( GL_DEPTH_TEST); 
////    glDepthFunc(GL_LEQUAL);
////    postVoxelRender(true); 
//
////    vlGLExt::glBindFramebufferEXT( GL_FRAMEBUFFER, 0 ); 
////    glMatrixMode( GL_MODELVIEW );
////    glPopMatrix();
////    glPixelTransferi( GL_MAP_COLOR, 1 ); 
////    if( !depth )
////        glDisable( GL_DEPTH_TEST); 
////    else
////        glEnable(GL_DEPTH_TEST);
////    if( !cull )
////        glDisable( GL_CULL_FACE ); 
////    else
////        glEnable( GL_CULL_FACE ); 
////    if( !blend)
////        glDisable( GL_BLEND); 
////    else
////        glEnable( GL_BLEND); 
////
////    glCullFace( cullface ); 
////    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3] ); 
////    glActiveTexture(GL_TEXTURE0); 
//}

//render well logs  color using shader
void VolumeRayCasterImp::renderWellLogsShadingPass()
{    
//    glEnable(GL_BLEND); 
//    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 
    
    //vmGLSmartView* glView = dynamic_cast<vmGLSmartView*>(GLContext::getCurrent()); 
    //vmGLSmartView* glView = this->myTornadoView; 
    //assert(glView); 
    //myWellLogManager ->setViewScale(Vect3f(glView ->getScale3dv()[0], glView ->getScale3dv()[1], glView ->getScale3dv()[2]) );
    while(vlGLExt::checkGLErrors("before renderWellLogsPreZPass"))
    {
        continue;
    }   
    
    vlGLExt::glBindFramebufferEXT( GL_FRAMEBUFFER, 0 ); 
    glEnable(GL_DEPTH_TEST); 
   // glDepthFunc( GL_LESS );
    glDepthFunc(GL_LEQUAL); 
    
    {
        //bind technique
        this->myWellLogManager->myRenderWellShader(this->myVolumeRayCasterAlgo->getDrawWellLogZPassTechnique(), false, false,  0); 
        //this->myWellLogManager-> render(0); 
    }
    
    glDepthFunc( GL_LESS );
    vlGLExt::checkGLErrors("after renderWellLogsPreZPass");
}
//render well logs  depth using shader
void VolumeRayCasterImp::renderWellLogsPreZPass()
{
    while(vlGLExt::checkGLErrors("before renderWellLogsPreZPass"))
    {
        continue;
    }
    vlGLExt::glBindFramebufferEXT(GL_FRAMEBUFFER, this->myVolumeRayCasterAlgo->getSinglePassDepthRT().fboHandle); 
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glEnable(GL_DEPTH_TEST); 
    
    glDepthFunc(GL_LEQUAL); 
    
    //glDisable(GL_CULL_FACE); 
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_FRONT);
    
    glEnable(GL_BLEND); 
    vlGLExt::glBlendEquation(GL_MAX);
    //b: true for mask, for ssao purpose, tricky
    glColorMask(false, true, true, false); 
//    
    {
        //bind technique
        this->myWellLogManager->myRenderWellShader(this->myVolumeRayCasterAlgo->getDrawWellLogZPassTechnique(), true, false, 0); 
    }
    
    vlGLExt::glBindFramebufferEXT(GL_FRAMEBUFFER, 0); 
    glEnable(GL_DEPTH_TEST); 
    glEnable(GL_CULL_FACE); 
    glCullFace(GL_BACK);
    glEnable(GL_BLEND); 
    vlGLExt::glBlendEquation(GL_FUNC_ADD); 
    glColorMask(true, true, true, true);  
    vlGLExt::checkGLErrors("after renderWellLogsPreZPass");
}

void VolumeRayCasterImp::renderEntryExit()
{
    this->myVolumeRayCasterAlgo->preRenderTornadoOpaque(); 
    
    renderWellLogsPreZPass(); 
    
    //here you can insert any more tornado object to render to z-buffer

    this->myVolumeRayCasterAlgo->postRenderTornadoOpaque();   
}

void VolumeRayCasterImp::inVoxelRender(timeval t)
{  
    if(this->myVolumeRayCasterAlgo->checkEarlyExit())
        return; 

    renderEntryExit(); 
    
    this->myVolumeRayCasterAlgo->render(t); 
}

void VolumeRayCasterImp::postVoxelRender( )
{ 
    
    bool enablePick = this ->myVolRenderConfig->isVoxelPickEnabled() ;
    this->myVolumeRayCasterAlgo->postRenderLoop(enablePick); 
    { 
        //cursor feed back 
        if(enablePick)
        { 
           const VoxelInfo* vpBuffer =  myVolumeRayCasterAlgo->getVoxelPickBuffer();
            
           BaseGeometry* geom = vlAppModel::getRef().getCurGeometry();

           Vect4d origin = geom->getOrigin(); 
           Vect4d lower = geom->getLower();
           Vect4d upper = geom->getUpper(); 

           Vect4d logicalPos(0,0,0,1); 
           logicalPos[0] = origin[0] + (upper[0] - lower[0] ) * vpBuffer->objPos[0] ; 
           logicalPos[1] = origin[1] + (upper[1] - lower[1] ) * vpBuffer->objPos[1] ; 
           logicalPos[2] = origin[2] + (upper[2] - lower[2] ) * vpBuffer->objPos[2] ; 

           Vect4d flin = geom->grd2lin(geom->log2grd(logicalPos));

           vmGLSmartView* glView = this->myTornadoView; 
           char buf[256]; 

           //        
           sprintf(buf, "Crossline: %d \t Inline: %d \t Depth: %d\t Seismic: %f \t Attribute: %f HasBrick: %s", 
                   (int)flin[0], (int)flin[1], (int)flin[2], vpBuffer->intensities[0], vpBuffer->intensities[1], 
                   vpBuffer->param[1] <= 0.01f ? "No" : "Yes" );

           vString status_msg;
           status_msg = vString(buf);
           Send( *glView, glView->pickedVoxelInfo, this ->myVolRenderConfig->getLastVoxelPickLoc(), &status_msg );           
        }
    }
}

bool VolumeRayCasterImp::initExtraTransferFunc(Volume *extraVol, Spectrum& extraVolSpect)
{
    while(vlGLExt::checkGLErrors("before initExtraTransferFunc"))
    {
        continue;
    }
    
    GLint align; 
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &align);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glPixelTransferi( GL_MAP_COLOR, 0 ); // turn off in case this was on
    
    Vect2d spec_range = extraVolSpect.getRange();
    Vect2d data_range = extraVolSpect.getDataRange ();
    if(data_range == Vect2d(0,1))
        data_range = extraVol->getLimit();
    
    myVolumeRayCasterAlgo ->setExtraVolDataTFRange(Vect2f( spec_range[0], spec_range[1] ),
    Vect2f(data_range[0], data_range[1]) ); 
   
    float* color1d = new float[NUM_GRADE * 4]; 
    float* opacitySAT = new float[NUM_GRADE];
    spectrum2array(extraVolSpect, color1d, opacitySAT); 
    
    GLuint& extraTF = myVolumeRayCasterAlgo ->getExtraTransferFunctionTexture(); 
    glBindTexture( GL_TEXTURE_1D,  extraTF); 
    glTexSubImage1D(GL_TEXTURE_1D, 0, 0, NUM_GRADE, GL_RGBA, GL_FLOAT, color1d);
    glBindTexture(GL_TEXTURE_1D, 0); 
    
    delete[] color1d; 
    delete[] opacitySAT; 
    
    return !vlGLExt::checkGLErrors("initExtraTransferFunc"); 
}

bool VolumeRayCasterImp::initTransferFunc(Volume* seisVol, Volume* attrVol, Spectrum& seisVolSpect, Spectrum& attrVolSpect )
{
    while(vlGLExt::checkGLErrors("before initTransferFunc"))
    {
        continue;
    }
    
    GLint align; 
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &align);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glPixelTransferi( GL_MAP_COLOR, 0 ); // turn off in case this was on
    
    if(seisVol > 0)
    {           
        int channelId; 
        channelId= this ->getTornadoVolumeChannel(seisVol); 
        
        OctreeUtils::VOXFORMAT volumeType = OctreeUtils::UNKNOWN; 
        volumeType = vlSparseVolumeOctreeWrapper::getTornadoVolType(seisVol);     
        assert(volumeType == OctreeUtils::FDM || volumeType == OctreeUtils::VG || volumeType == OctreeUtils::CAPISINGLE); 
         
        Vect2d spec_range = seisVolSpect.getRange();
        Vect2d data_range = seisVolSpect.getDataRange ();
        if(data_range == Vect2d(0,1))
            data_range = seisVol->getLimit();
        
        this->myVolumeRayCasterAlgo->setSeisVolDataTFRange(
               Vect2f( spec_range[0], spec_range[1] ), 
               Vect2f(data_range[0], data_range[1])  ); 
        
        float* color1d = new float[NUM_GRADE * 4]; 
        float* opacitySAT = new float[NUM_GRADE];
        spectrum2array(seisVolSpect, color1d, opacitySAT); 
        

        GLuint& allTF = this->myVolumeRayCasterAlgo->getTransferFunctionTextureArray(); 
        glBindTexture( GL_TEXTURE_1D_ARRAY,  allTF); 
        glTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, channelId, NUM_GRADE, 1, GL_RGBA, GL_FLOAT, color1d);
        glBindTexture(GL_TEXTURE_1D_ARRAY, 0); 
        
        GLuint& sat = this ->myVolumeRayCasterAlgo->getOpacitySATArray(); 
        glBindTexture( GL_TEXTURE_1D_ARRAY,  sat); 
        glTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, channelId, NUM_GRADE, 1, GL_RED, GL_FLOAT, opacitySAT);
        glBindTexture(GL_TEXTURE_1D_ARRAY, 0);         
        delete[] color1d; 
        delete[] opacitySAT; 
    }
    
    if(attrVol > 0)
    {    
        int channelId; 
        channelId= this ->getTornadoVolumeChannel(attrVol);
        
        OctreeUtils::VOXFORMAT volumeType = OctreeUtils::UNKNOWN; 
        volumeType = vlSparseVolumeOctreeWrapper::getTornadoVolType(attrVol);     
        assert(volumeType == OctreeUtils::FDM || volumeType == OctreeUtils::VG || volumeType == OctreeUtils::CAPISINGLE); 
         
        Vect2d spec_range = attrVolSpect.getRange();
        Vect2d data_range = attrVolSpect.getDataRange ();
        if(data_range == Vect2d(0,1))
            data_range = attrVol->getLimit();
        
        if(channelId == 0 )
        {
            this->myVolumeRayCasterAlgo->setSeisVolDataTFRange(
               Vect2f( spec_range[0], spec_range[1] ), 
               Vect2f(data_range[0], data_range[1])  ); 
        }
        else
        {
            this->myVolumeRayCasterAlgo->setAttrVolDataTFRange
            (
               Vect2f( spec_range[0], spec_range[1] ), Vect2f(data_range[0], data_range[1])  );           
        }
        float* color1d = new float[NUM_GRADE * 4]; 
        float* opacitySAT = new float[NUM_GRADE];
        spectrum2array(attrVolSpect, color1d, opacitySAT); 
        
        GLuint& allTF = myVolumeRayCasterAlgo ->getTransferFunctionTextureArray(); 
        glBindTexture( GL_TEXTURE_1D_ARRAY,  allTF); 
        glTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, channelId, NUM_GRADE, 1, GL_RGBA, GL_FLOAT, color1d);
        glBindTexture(GL_TEXTURE_1D_ARRAY, 0);    
        
        GLuint& sat = myVolumeRayCasterAlgo ->getOpacitySATArray(); 
        glBindTexture( GL_TEXTURE_1D_ARRAY,  sat); 
        glTexSubImage2D(GL_TEXTURE_1D_ARRAY, 0, 0, channelId, NUM_GRADE, 1, GL_RED, GL_FLOAT, opacitySAT);
        glBindTexture(GL_TEXTURE_1D_ARRAY, 0); 

        delete[] color1d; 
        delete[] opacitySAT; 
    }     
    glPixelTransferi( GL_MAP_COLOR, 1 ); 
    glPixelStorei (GL_UNPACK_ALIGNMENT, align);
    
    myVolumeRayCasterAlgo->setEnvelopeDirty(true); 
    
    return !vlGLExt::checkGLErrors("initTransferFunc"); 
}

//grid texture is for empty space skipping 
void VolumeRayCasterImp::computeGridTexture(Volume* torVolCh1, Volume* torVolCh2, Vect3i gridDim, Vect3i voxelsPerCell )
{
    //1. create grid texture : myGridDim Vect3i
    //2. create a volumetex from disk: uint16_t. xyz: 
    //3: check to see
    if(torVolCh1 > 0 && torVolCh2 > 0)
        assert( torVolCh1->getSize() == torVolCh2->getSize() ); 
    
    while(vlGLExt::checkGLErrors("before computeGridTexture"))
    {
        continue;
    }
    
    if(this ->myOctreeVol == NULL)
        return ; 
    
    int numVoxels = gridDim[0] * gridDim[1] * gridDim[2];
    
    float* gridVolume = new float[numVoxels * 4]; 
    
    OctreeUtils::VOXFORMAT volumeTypeCh1 = OctreeUtils::UNKNOWN; 
    OctreeUtils::VOXFORMAT volumeTypeCh2 = OctreeUtils::UNKNOWN; 
    
    if(torVolCh1)
    {
        volumeTypeCh1 = vlSparseVolumeOctreeWrapper::getTornadoVolType(torVolCh1);  
        assert(volumeTypeCh1 == OctreeUtils::VG || volumeTypeCh1 == OctreeUtils::FDM || volumeTypeCh1 == OctreeUtils::CAPISINGLE); 
    }
    if(torVolCh2)
    {
        volumeTypeCh2 = vlSparseVolumeOctreeWrapper::getTornadoVolType(torVolCh2);  
        assert(volumeTypeCh2 == OctreeUtils::VG || volumeTypeCh2 == OctreeUtils::FDM || volumeTypeCh1 == OctreeUtils::CAPISINGLE); 
    }
    
    for(int z = 0; z < gridDim[2]; z++)
    {
        #pragma omp parallel for 
        for(int threadID = 0; threadID <  omp_get_num_procs(); threadID++ )
        {    
             for(int y = threadID; y < gridDim[1]; y+= omp_get_num_procs())
             {
                 for(int x = 0; x < gridDim[0]; x++)
                 {
                     Vect3i ptGridToPos(x, y, z); 
                     Vect3i ptVolumeToPos = ptGridToPos * voxelsPerCell; 
                     Vect3i ptVolumeToMinPos = OctreeUtils::max3i(ptVolumeToPos - Vect3i(1,1,1), Vect3i(0)); 
                     Vect3i ptVolumeToMaxPos = OctreeUtils::min3i(ptVolumeToPos + voxelsPerCell + Vect3i(1,1,1), myOctreeVol->getVolumeDimension() ); 

                     float fmax_c1 = FLT_MIN;
                     float fmin_c1 = FLT_MAX;
                     float fmax_c2 = FLT_MIN; 
                     float fmin_c2 = FLT_MAX; 
                     
                     for(int k = ptVolumeToMinPos[2]; k <  ptVolumeToMaxPos[2]; k++)
                         for(int j = ptVolumeToMinPos[1]; j <  ptVolumeToMaxPos[1]; j++)
                             for(int i = ptVolumeToMinPos[0]; i <  ptVolumeToMaxPos[0]; i++)
                             {
                                 if(torVolCh1 && volumeTypeCh1 == OctreeUtils::VG)
                                 {
                                    fmax_c1 = max(fmax_c1, (float)torVolCh1->getCube()->getVoxelValue( i, j, k )); 
                                    fmin_c1 = min(fmin_c1, (float)torVolCh1->getCube()->getVoxelValue( i, j, k ));
                                 }
                                 
                                 if(torVolCh1 && ( volumeTypeCh1 == OctreeUtils::FDM || volumeTypeCh1 == OctreeUtils::CAPISINGLE) )
                                 {
                                    fmax_c1 = max(fmax_c1, (float)torVolCh1->getCube()->getVoxelValue( i, j, k )); 
                                    fmin_c1 = min(fmin_c1, (float)torVolCh1->getCube()->getVoxelValue( i, j, k ));                                     
                                 }
                                 
                                 if(torVolCh2 && volumeTypeCh2 == OctreeUtils::VG)
                                 {
                                    fmax_c2 = max(fmax_c2, (float)torVolCh2->getCube()->getVoxelValue( i, j, k )); 
                                    fmin_c2 = min(fmin_c2, (float)torVolCh2->getCube()->getVoxelValue( i, j, k ));
                                 }
                                 
                                 if(torVolCh2 && ( volumeTypeCh2 == OctreeUtils::FDM || volumeTypeCh2 == OctreeUtils::CAPISINGLE) )
                                 {
                                    fmax_c2 = max(fmax_c2, (float)torVolCh2->getCube()->getVoxelValue( i, j, k )); 
                                    fmin_c2 = min(fmin_c2, (float)torVolCh2->getCube()->getVoxelValue( i, j, k ));                                     
                                 }
                             }
                     
                    size_t offset_c1, offset_c2; 
                    if(torVolCh1)
                    {
                        offset_c1 = ( volumeTypeCh1 == OctreeUtils::VG ? (x + y* gridDim[0] + (gridDim[2] - z - 1) * gridDim[1]*gridDim[0])
                             : (x + y* gridDim[0] + z * gridDim[1]*gridDim[0]) ); 
                                     
                        gridVolume[ offset_c1 * 4  ] = fmin_c1 ; // vox
                        gridVolume[ offset_c1 * 4 + 1 ] = fmax_c1 ; // vox
                    }
                    
                    if(torVolCh2)
                    {
                        offset_c2 = ( volumeTypeCh2 == OctreeUtils::VG ? (x + y* gridDim[0] + (gridDim[2] - z - 1) * gridDim[1]*gridDim[0])
                             : (x + y* gridDim[0] + z * gridDim[1]*gridDim[0]) ); 
                                     
                        gridVolume[ offset_c2 * 4 + 2 ] = fmin_c2 ; // vox
                        gridVolume[ offset_c2 * 4 + 3 ] = fmax_c2 ; // vox
                    }                    
                 }
             }
        }
    }   

    if(myGridTexture3D == 0)
    {
        glGenTextures( 1, &myGridTexture3D); 
    }
    
    GLint align; 
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &align);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glPixelTransferi( GL_MAP_COLOR, 0 ); // turn off in case this was on
    
    glBindTexture( GL_TEXTURE_3D, myGridTexture3D );    
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER ); 
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER );      
    const GLfloat boarderColor[4] = {0,0,0,0}; 
    glTexParameterfv( GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, &boarderColor[0]  );
    
    vlGLExt::glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, gridDim[0], gridDim[1], gridDim[2], 0, GL_RGBA, GL_FLOAT, gridVolume);    
    //glFinish();  

    glBindTexture( GL_TEXTURE_3D, 0 );    
    glPixelTransferi( GL_MAP_COLOR, 1 ); 
    glPixelStorei (GL_UNPACK_ALIGNMENT, align);   
    
     
    vlGLExt::checkGLErrors("after computeGridTexture");
    delete[] gridVolume;  
}

void VolumeRayCasterImp::invalidateGridBuffer()
{
   
}

void VolumeRayCasterImp::invalidateGridTexture()
{
    vlGLExt::glDeleteTextures(1, &myGridTexture3D); 
    myVolumeRayCasterAlgo->updateGridTexture(0);
    //test if needed glFinish(); 
    glFinish(); 
}

void VolumeRayCasterImp::invalidateTransferFunction()
{
    this ->myTransferFunctionInitialized = false; 
}

void VolumeRayCasterImp::invalidateExtraVolTransferFunction()
{
    this->myExtraVolumeTransferFunctionInitialized = false; 
}

//test purpose, draw clipping hrz texture to screen. 
void VolumeRayCasterImp::drawHrzTextureToScreen(int hid, Vect4i viewportSize)
{
    drawTextureToScreen(myVolumeRayCasterAlgo->getClippingHrzTex2D(hid), viewportSize); 
}

void VolumeRayCasterImp::drawSSAOTextureToScreen(Vect4i viewportSize)
{
    drawTextureToScreen(myVolumeRayCasterAlgo->getSSAOTex2D(), viewportSize); 
}
    
void VolumeRayCasterImp::drawSSAOBlurredTextureToScreen(Vect4i viewportSize)
{
    drawTextureToScreen(myVolumeRayCasterAlgo->getSSAOBlurredTex2D(), viewportSize);
}

void VolumeRayCasterImp::drawTextureToScreen(const GLuint& texHandle, const Vect4i viewportSize)
{
    GLint viewport_bk[4];
    glGetIntegerv( GL_VIEWPORT, viewport_bk );
    
    glDisable(GL_DEPTH_TEST);
    glViewport(viewportSize[0], viewportSize[1], viewportSize[2], viewportSize[3]); 
  
    vlGLExt::glBindFramebufferEXT( GL_FRAMEBUFFER,  0); 
    this->myVolumeRayCasterAlgo->getFullScreenCopyTex2dTechnique()->bindPipeline(); 
    vlGLExt::glActiveTexture(GL_TEXTURE0); 
    glBindTexture(GL_TEXTURE_2D, texHandle);
    vlGLExt::glBindVertexArray(0); 
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    myVolumeRayCasterAlgo->getFullScreenCopyTex2dTechnique()->unbindPipeline();   
    glBindTexture(GL_TEXTURE_2D, 0);
    glViewport(viewport_bk[0],viewport_bk[1], viewport_bk[2], viewport_bk[3]);     
    
    glEnable(GL_DEPTH_TEST);
}
//output 1d texture array for transferfunction and sat opacity(useful for empty space skipping)
void VolumeRayCasterImp::spectrum2array(Spectrum& spectrum, float*& array, float*& opacitySAT)
{
    assert(array && opacitySAT); 
    unsigned short* rmap = spectrum.RMap();
    unsigned short* gmap = spectrum.GMap();
    unsigned short* bmap = spectrum.BMap();
    unsigned short* amap = spectrum.AMap();
    unsigned short* afreemap = spectrum.AFreeMap();
    
    float r, g, b, a, fa;
    
    for (int i = 0; i < NUM_GRADE; i++) {
        r = (float) rmap[i];
        g = (float) gmap[i];
        b = (float) bmap[i];
        a = (float) amap[i];
        fa = (float) afreemap[i];

        r /= 65535.0f;
        g /= 65535.0f;
        b /= 65535.0f;
        a /= 65535.0f;
        fa /= 65535.0f;

        array[4*i] = r;
        array[4*i+1] = g;
        array[4*i+2] = b;
        array[4*i+3] = a;
        
        if(i == 0)
          opacitySAT[i] = a; 
        else
          opacitySAT[i] = a + opacitySAT[i - 1]; 
    }  
}


int VolumeRayCasterImp::getTornadoVolumeChannel(Volume* delVol) const
{
    if(this ->myOctreeVol)
    {
        vlSparseVolumeOctreeWrapper* tmpOctreeVol = dynamic_cast<vlSparseVolumeOctreeWrapper*>(myOctreeVol); 
        if(tmpOctreeVol)
        {
            return tmpOctreeVol->queryTornadoVolChannel(delVol);
        }
        return -1; 
    }
    return -1; 
}

bool VolumeRayCaster::blockRendering = false; 
bool VolumeRayCaster::blockModify = false; 
bool VolumeRayCaster::blockUpdate = false; 
bool VolumeRayCaster::blockDelete = false; 

VolumeRayCaster::VolumeRayCaster( vmGLSmartView* torView, VolumeSlateManager* vsm) : VolumeRayCasterDummy(torView, vsm)
{
    myImp = new VolumeRayCasterImp(this);
    myImp->setTornadoView(torView); 
    myImp->setVolumeSlateManager(vsm); 
    
    int w, h; 
    torView->getSize(w, h); 
    resize( w, h ); 
    myImp->connectSignal();  
}

VolumeRayCaster::~VolumeRayCaster()
{
    myImp->disconnectSignal();
    delete myImp;
}

void VolumeRayCaster::resize( int x, int y)
{
    myImp->myVolumeRayCasterAlgo->resize(x, y); 
}

bool VolumeRayCaster ::isUseShaderRenderSlice() const
{
    return myImp ->myVolRenderConfig->isUseShaderRenderSlice(); 
}

void  VolumeRayCaster :: renderSlices()
{
    //render all slices by using shader for testing purpose
    myImp ->myVolumeRayCasterAlgo->renderSlices(Vect3f(0), Vect3i(0), Vect3f(0), Vect3f(0), Vect3i(0), Vect3f(0));      
}
void VolumeRayCaster::render(timeval t)
{
    if(blockRendering)
    {
        return ;
    }
    blockModify = true; 
    
    //myImp ->drawLights(); 
    
    if(!myImp ->preVoxelRender())
        return; 
    
    myImp->preRenderGLState();
    
    myImp ->inVoxelRender(t); 
    
    myImp ->postVoxelRender(); 
    
    myImp->postRenderGLState();
     
    if(myImp->myVisualizeHrzImage)
    {
        myImp->drawHrzTextureToScreen(0, Vect4i(0,0,800,600)); 
    }
    if(myImp->myVisualizeSSAOImage)
    {
        myImp->drawSSAOTextureToScreen(Vect4i(0,0,800,600)); 
    }
    if(myImp->myVisualizeSSAOBlurredImage)
    {
        myImp->drawSSAOBlurredTextureToScreen(Vect4i(0,0,800,600)); 
    }
}
//regular mode
bool VolumeRayCaster::exportSVO2LDM(Volume *seisVol, Volume *attrVol,  const vString& dir) 
{
    vlSparseVolumeOctreeWrapper* svowrapper = dynamic_cast<vlSparseVolumeOctreeWrapper*>(myImp->myOctreeVol);
    if(svowrapper)
    {
        if(!vUtl::directoryExists(dir.c_str() ) )
        {
            vUtl::mkdirRecursive (dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
        
        return svowrapper->exportLDM(seisVol, attrVol, dir, false, myImp->mySVOBuildProgress);
    }
    else
        return false; 
}
//rgba
bool VolumeRayCaster::exportSVO2LDM(vector<Volume*>& channelVols, const vString& dir)
{
    vlSparseVolumeOctreeWrapper* svowrapper = dynamic_cast<vlSparseVolumeOctreeWrapper*>(myImp->myOctreeVol);
    if(svowrapper)
    {
        if(!vUtl::directoryExists(dir.c_str() ) )
        {
            vUtl::mkdirRecursive (dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        }
        
        return svowrapper->exportLDM(dir, false, myImp->mySVOBuildProgress);
    }
    else
        return false; 
}

void VolumeRayCaster::deleteTriangleMeshGroup(TriangleMeshGroup *p)
{
    if( p == myImp->myTriangleMeshShaderViewG.getMeshG()  )
      myImp->myTriangleMeshShaderViewG.setMeshG (NULL);
}

void VolumeRayCaster::newTriangleMeshGroup(TriangleMeshGroup *p)
{
    myImp->myTriangleMeshShaderViewG.setMeshG (p);
}
    
//A lot of resource needs to updated, might missing something here
void VolumeRayCaster::onNewCurrentSeismic (Volume *vol)
{
    myImp->myVolumeRayCasterAlgo->invalidateStreamSliceTexture(); 
}

void VolumeRayCaster::onNewCurrentAttribute (Volume *vol)
{
    myImp->myVolumeRayCasterAlgo->invalidateStreamSliceTexture(); 
}


void VolumeRayCaster::onNewCurrentReferenceGeometry(BaseGeometry* geom)
{
     myImp->myVolumeRayCasterAlgo->invalidateStreamSliceTexture(); 
}

void VolumeRayCaster::onUpdateGlLightProperty(float* ambientCL, float* diffuseCL, float* specularCL, int lightIdx, double* lightDir)
{  
    if(lightIdx == 0)
    {
        Vect4f ambientCL4fv(ambientCL[0], ambientCL[1], ambientCL[2], ambientCL[3]);
        Vect4f diffuseCL4fv(diffuseCL[0], diffuseCL[1], diffuseCL[2], diffuseCL[3]);
        Vect4f specularCL4fv(specularCL[0], specularCL[1], specularCL[2], specularCL[3]);
       
        myImp->myVolumeRayCasterAlgo->setLightProperty(ambientCL4fv, diffuseCL4fv, specularCL4fv);   
    }
}

void VolumeRayCaster::onNewBasicSliceVis(int axis, bool vis)
{
    if(axis >= 0 && axis < 3)
    {
        myImp->myVolumeRayCasterAlgo->setBasicSliceVis(axis, vis); 
    }
}

void VolumeRayCaster::onRefreshBaseSliceChannel(SliceBase* refreshSlice)
{
    //search the hash map to find the slatebase
    if(this->myImp->mySliceBaseListenerCache.count(refreshSlice) > 0)
    {
        //flag need to be refreshed
       myImp->mySliceBaseListenerCache[refreshSlice].second = true; 
    }
    else
    {
        //some unsync between tornado slice and scene graph node happens. bug 
        assert(false); 
    }
}
//could be optimized, very clumsy
void VolumeRayCaster::RefreshBaseSliceChannel(SliceBase* refreshSlice)
{
    //search the hash map to find the slatebase
    if(this->myImp->mySliceBaseListenerCache.count(refreshSlice) > 0)
    {
        SlateBase* attachedSlate = myImp->mySliceBaseListenerCache[refreshSlice].first;
        
        Volume* activeVolSVO = NULL; 
        Volume* extraTorVol = NULL; 
        myImp->getSVOVolumeState(activeVolSVO, extraTorVol);
        
        bool isShowSeis = myImp->myVSM->getAllSeismicSlicesVis();
        bool isShowAttr = myImp->myVSM->getAllAttributeSlicesVis();
        
        //find type and id within that type; 
        int baseslateIdx = myImp->myVSM->isBasicSlate(*attachedSlate); 
        int tslateIdx = myImp->myVSM->getSlateIndex(*attachedSlate);
        int fslateIdx = myImp->myVSM->getFenceIndex(*attachedSlate);
        int fsetIdx = myImp->myVSM->getFenceSet(*attachedSlate);

        if(extraTorVol)
        {
            int volIdx = attachedSlate->getVolumeIndex(*(extraTorVol));
            SliceBase* channelSlice = attachedSlate->getSlice(volIdx); 
            if(channelSlice == refreshSlice)
            {
                ColorSlice* torSlice = dynamic_cast<ColorSlice*>(refreshSlice); 
                if(torSlice && !torSlice->isInvalid())
                {          
                    Vect4d bgn_vol, end_vol;
                    int axis_h; 
                    torSlice->getDefin( axis_h, bgn_vol, end_vol ); //survey coord, 
                    Vect2i slc_size = extraTorVol->getSliceSize( bgn_vol, end_vol );
                    Matrixd l2g_vol;
                    //cube
                    Vect4d cbgn_vol, cend_vol;
                    l2g_vol = extraTorVol->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID);      
                    cbgn_vol = l2g_vol * bgn_vol;
                    cend_vol = l2g_vol * end_vol;

                    float* tmp = new float[slc_size[0] * slc_size[1]];
                    extraTorVol->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                    assert(tmp); 
                    if(baseslateIdx >= 0)
                    {
                        myImp->myVolumeRayCasterAlgo->updateBasicSliceNode(baseslateIdx, 2, cbgn_vol, cend_vol, tmp, slc_size, 
                            attachedSlate->isVisible(),  isShowSeis, isShowAttr ); 
                    }
                    else if(tslateIdx >= 0)
                    {
                        myImp->myVolumeRayCasterAlgo->updateTraverseSliceNode(tslateIdx, 2, cbgn_vol, cend_vol, tmp, slc_size, 
                            attachedSlate->isVisible(),  isShowSeis, isShowAttr ); 
                    }
                    else if(fslateIdx >= 0 )
                    {
                        //current fence
                        myImp->myVolumeRayCasterAlgo->updateFenceSliceNode(fsetIdx, fslateIdx, 2, cbgn_vol, cend_vol, tmp, slc_size, 
                            attachedSlate->isVisible(),  isShowSeis, isShowAttr );                     
                    }
                    else if(fsetIdx >= 0)
                    {
                        //not current fence set
                        fslateIdx = myImp->myVSM->getFenceIndex(*attachedSlate, fsetIdx); 
                        myImp->myVolumeRayCasterAlgo->updateFenceSliceNode(fsetIdx, fslateIdx, 2, cbgn_vol, cend_vol, tmp, slc_size, 
                            attachedSlate->isVisible(),  isShowSeis, isShowAttr ); 
                    }
                    else
                    {
                        assert(false); 
                    }
                }    
            }
        }

        if(activeVolSVO)
        {
            int volIdx = attachedSlate->getVolumeIndex(*(activeVolSVO));
            SliceBase* channelSlice = attachedSlate->getSlice(volIdx); 
            if(channelSlice == refreshSlice)
            {
                ColorSlice* torSlice = dynamic_cast<ColorSlice*>(refreshSlice); 
                if( torSlice && !torSlice ->isInvalid())
                {
                    Vect4d bgn_vol, end_vol;
                    int axis_h; 
                    torSlice->getDefin( axis_h, bgn_vol, end_vol ); //survey coord, 
                    Vect2i slc_size = activeVolSVO->getSliceSize( bgn_vol, end_vol );
                    Matrixd l2g_vol;
                    //cube
                    Vect4d cbgn_vol, cend_vol;
                    l2g_vol = activeVolSVO->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID);      
                    cbgn_vol = l2g_vol * bgn_vol;
                    cend_vol = l2g_vol * end_vol;
                    float* tmp = new float[slc_size[0] * slc_size[1]];
                    activeVolSVO->makeSliceN2P( bgn_vol, end_vol, 32, tmp);  
                    assert(tmp); 
                    if(baseslateIdx >= 0)
                    {
                        myImp->myVolumeRayCasterAlgo->updateBasicSliceNode(baseslateIdx, 0, cbgn_vol, cend_vol, tmp, slc_size, 
                            attachedSlate->isVisible(),  isShowSeis, isShowAttr ); 
                    }
                    else if(tslateIdx >= 0)
                    {
                        myImp->myVolumeRayCasterAlgo->updateTraverseSliceNode(tslateIdx, 0, cbgn_vol, cend_vol, tmp, slc_size, 
                            attachedSlate->isVisible(),  isShowSeis, isShowAttr ); 
                    }
                    else if(fslateIdx >= 0 )
                    {
                        myImp->myVolumeRayCasterAlgo->updateFenceSliceNode(fsetIdx, fslateIdx, 0, cbgn_vol, cend_vol, tmp, slc_size, 
                            attachedSlate->isVisible(),  isShowSeis, isShowAttr );                     
                    }
                    else if(fsetIdx >= 0)
                    {
                        //not current fence set
                        fslateIdx = myImp->myVSM->getFenceIndex(*attachedSlate, fsetIdx); 
                        myImp->myVolumeRayCasterAlgo->updateFenceSliceNode(fsetIdx, fslateIdx, 0, cbgn_vol, cend_vol, tmp, slc_size, 
                            attachedSlate->isVisible(),  isShowSeis, isShowAttr ); 
                    }
                    else
                    {
                        assert(false); 
                    }

                    if( myImp->myOctreeVol->getNumVolumes() > 1)
                    {       
                        float* tmp = new float[slc_size[0] * slc_size[1]];
                        if( activeVolSVO == myImp->myAttrVol ) 
                        {
                            myImp->myAttrVol->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                        }
                        else
                        {
                            myImp->mySeisVol->makeSliceN2P( bgn_vol, end_vol, 32, tmp);
                        }
                        assert(tmp); 
                        if(baseslateIdx >= 0)
                        {
                            myImp->myVolumeRayCasterAlgo->updateBasicSliceNode(baseslateIdx, 1, cbgn_vol, cend_vol, tmp, slc_size, 
                                attachedSlate->isVisible(),  isShowSeis, isShowAttr ); 
                        }
                        else if(tslateIdx >= 0)
                        {
                            myImp->myVolumeRayCasterAlgo->updateTraverseSliceNode(tslateIdx, 1, cbgn_vol, cend_vol, tmp, slc_size, 
                                attachedSlate->isVisible(),  isShowSeis, isShowAttr ); 
                        }
                        else if(fslateIdx >= 0 )
                        {
                            myImp->myVolumeRayCasterAlgo->updateFenceSliceNode(fsetIdx, fslateIdx, 1, cbgn_vol, cend_vol, tmp, slc_size, 
                                attachedSlate->isVisible(),  isShowSeis, isShowAttr );                     
                        }
                        else if(fsetIdx >= 0)
                        {
                            //not current fence set
                            fslateIdx = myImp->myVSM->getFenceIndex(*attachedSlate, fsetIdx); 
                            myImp->myVolumeRayCasterAlgo->updateFenceSliceNode(fsetIdx, fslateIdx, 1, cbgn_vol, cend_vol, tmp, slc_size, 
                                attachedSlate->isVisible(),  isShowSeis, isShowAttr ); 
                        }
                        else
                        {
                            assert(false); 
                        }
                    }
                }
            }
        }
        if(baseslateIdx >= 0)
        {
            myImp->myVolumeRayCasterAlgo->attachBasicSliceNodeExtraVolChannel(baseslateIdx, extraTorVol!=NULL); 
        }
        else if(tslateIdx >= 0)
        {
            myImp->myVolumeRayCasterAlgo->attachTraverseSliceNodeExtraVolChannel(tslateIdx, extraTorVol!=NULL); 
        }
        else if(fslateIdx >= 0 )
        {
             myImp->myVolumeRayCasterAlgo->attachFenceSliceNodeExtraVolChannel(fsetIdx, fslateIdx, extraTorVol!=NULL);                
        }
        else
        {
            assert(false); 
        }
    }
}

void VolumeRayCaster::onRefreshCurvedSliceChannel(CurvedSlice* refreshSlice)
{
    //search the hash map to find the slatebase
    if(this->myImp->myCurvedSliceListenerCache.count(refreshSlice) > 0)
    {
       myImp->myCurvedSliceListenerCache[refreshSlice].second = true; 
    }
    else
    {
        assert(false); 
    }
}

void VolumeRayCaster::RefreshCurvedSliceChannel(CurvedSlice* refreshSlice)
{
    //search the hash map to find the slatebase
    if(this->myImp->myCurvedSliceListenerCache.count(refreshSlice) > 0)
    {
        CurvedSlate* attachedSlate = myImp->myCurvedSliceListenerCache[refreshSlice].first; 
        
        Volume* activeVolSVO = NULL; 
        Volume* extraTorVol = NULL; 
        myImp->getSVOVolumeState(activeVolSVO, extraTorVol);
        
        bool isShowSeis = myImp->myVSM->getAllSeismicSlicesVis();
        bool isShowAttr = myImp->myVSM->getAllAttributeSlicesVis();
        
        int cs_Idx = myImp->myVSM->getCurvedSlateIndex(attachedSlate);
        if(extraTorVol)
        {
            int volIdx = attachedSlate->getVolumeIndex(*(extraTorVol));         
            CurvedSlice* curvedSlice = attachedSlate->getCurvedSlice(volIdx); 
            if(curvedSlice == refreshSlice)
            {   
                vector<Vect4d> pts = curvedSlice->getCurveDrawPts();
                vector<float*> traces = curvedSlice->getCurveDrawTraces();
                
                myImp->myVolumeRayCasterAlgo->updateCurvedFenceSliceNode( cs_Idx, 2, pts, traces, 
                        curvedSlice->getCurveZOrig(), 
                        curvedSlice->getCurveZStep(), 
                        curvedSlice->getCurveZNumSamples(), 
                        extraTorVol->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID), 
                        curvedSlice->isVisible(), isShowSeis, isShowAttr ); 
            }             
        }
        if(activeVolSVO)
        {
            int volIdx = attachedSlate->getVolumeIndex(*(activeVolSVO));
            CurvedSlice* curvedSlice = attachedSlate->getCurvedSlice(volIdx); 
            if(curvedSlice == refreshSlice)
            {
                vector<Vect4d> pts = curvedSlice->getCurveDrawPts();
                vector<float*> traces = curvedSlice->getCurveDrawTraces();
                myImp->myVolumeRayCasterAlgo->updateCurvedFenceSliceNode(cs_Idx, 0, pts, traces, 
                        curvedSlice->getCurveZOrig(), 
                        curvedSlice->getCurveZStep(), 
                        curvedSlice->getCurveZNumSamples(), 
                        activeVolSVO->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID), 
                        curvedSlice->isVisible(), isShowSeis, isShowAttr ); 
                
                if( myImp->myOctreeVol->getNumVolumes() > 1)
                {
                    int volIdx; 
                    Volume* coRenderVolume; 
                    if( activeVolSVO == myImp->mySeisVol ) 
                    {
                        coRenderVolume = myImp->myAttrVol; 
                    }
                    else
                    {
                        coRenderVolume = myImp->mySeisVol;
                    }
                    volIdx = attachedSlate -> getVolumeIndex(*coRenderVolume);
                    CurvedSlice* curvedSlice = attachedSlate->getCurvedSlice(volIdx); 
                    if(curvedSlice)
                    {
                        vector<Vect4d> pts = curvedSlice->getCurveDrawPts();
                        vector<float*> traces = curvedSlice->getCurveDrawTraces();
                        myImp->myVolumeRayCasterAlgo->updateCurvedFenceSliceNode(cs_Idx, 1, pts, traces, 
                                curvedSlice->getCurveZOrig(), 
                                curvedSlice->getCurveZStep(), 
                                curvedSlice->getCurveZNumSamples(), 
                                coRenderVolume->calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID), 
                                curvedSlice->isVisible(), isShowSeis, isShowAttr );     
                    } 
                }
            }        
        }
        myImp->myVolumeRayCasterAlgo->attachCurvedFenceNodeExtraVolChannel(cs_Idx, extraTorVol!=NULL); 
    }
}

void VolumeRayCaster::onPostMakeRotatedSlate(SlateBase* traverseSlate)
{
    //connect signal, and cache it
    Volume* activeVolSVO = NULL; 
    Volume* extraTorVol = NULL; 
    myImp->getSVOVolumeState(activeVolSVO, extraTorVol);
    
    char sliceName[128]; 
    int traverseId = myImp->myVSM->getSlateIndex(*traverseSlate); 
    vector<vlSceneGraphNode*> nodes;
    sprintf(sliceName, "Traverse_%d", traverseId);
    vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
    if(nodes.size() == 0 )
    {            
        vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
        vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode); 
        OrthoSliceRenderable* orthoslice = new OrthoSliceRenderable(); 
        newNode->addComponent(orthoslice);     
    }

    if(extraTorVol)
    {
        int volIdx = traverseSlate->getVolumeIndex(*(extraTorVol));
        SliceBase* channelSlice = traverseSlate->getSlice(volIdx); 
        if(myImp->mySliceBaseListenerCache.count(channelSlice) == 0)
        { 
            Connect( *channelSlice, channelSlice->Refresh, *this, &VolumeRayCaster::onRefreshBaseSliceChannel );    
            myImp->mySliceBaseListenerCache.insert(make_pair(channelSlice, make_pair(traverseSlate,true) )); 
            //onRefreshBaseSliceChannel(channelSlice);
        }
        else
        {
            assert(false); 
        }
    }
    if(activeVolSVO)
    {
        int volIdx = traverseSlate->getVolumeIndex(*(activeVolSVO));
        SliceBase* channelSlice = traverseSlate->getSlice(volIdx); 
        if(myImp->mySliceBaseListenerCache.count(channelSlice) == 0)
        { 
            Connect( *channelSlice, channelSlice->Refresh, *this, &VolumeRayCaster::onRefreshBaseSliceChannel );    
            myImp->mySliceBaseListenerCache.insert(make_pair(channelSlice, make_pair(traverseSlate,true) )); 
            //onRefreshBaseSliceChannel(channelSlice);
        }
        else
        {
            assert(false); 
        }
    }
}

void VolumeRayCaster::onPreDeleteRotatedSlate(SlateBase* travseSlate)
{
    int id = myImp->myVSM->getSlateIndex(*travseSlate);
    if(id >= 0 )
    {
        myImp->myVolumeRayCasterAlgo->deleteTraverseSliceNode(id); 
    }
    
    for(int i = 0; i < travseSlate->getSlicesNum(); i++)
    {
        SliceBase* slc = travseSlate-> getSlice(i); 
        if( myImp->mySliceBaseListenerCache.count(slc) > 0 )
        {
            Disconnect( *slc, slc->Refresh, *this, &VolumeRayCaster::onRefreshBaseSliceChannel );     
            myImp->mySliceBaseListenerCache.erase(slc); 
        }    
    }
}

void VolumeRayCaster::onPostMakeFenceSlate(SlateBase* fenceSlate)
{
    //connect signal, and cache it
    Volume* activeVolSVO = NULL; 
    Volume* extraTorVol = NULL; 
    myImp->getSVOVolumeState(activeVolSVO, extraTorVol);
    
    char sliceName[128]; 
    int fenceId = myImp->myVSM->getFenceIndex(*fenceSlate); 
    int fenceSetId = myImp->myVSM->getFenceSet(*fenceSlate);
    
    vector<vlSceneGraphNode*> nodes;
    sprintf(sliceName, "Fence_%d_%d", fenceSetId, fenceId);
    vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
    if(nodes.size() == 0 )
    {            
        fprintf(stderr, "new vlSceneGraphNode: %s\n", sliceName);
        vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
        vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode); 
        OrthoSliceRenderable* orthoslice = new OrthoSliceRenderable(); 
        newNode->addComponent(orthoslice);     
    }
    else
    {
        assert(false); 
    }
    
    if(extraTorVol)
    {
        int volIdx = fenceSlate->getVolumeIndex(*(extraTorVol));
        SliceBase* channelSlice = fenceSlate->getSlice(volIdx); 
        if(myImp->mySliceBaseListenerCache.count(channelSlice) == 0)
        { 
            Connect( *channelSlice, channelSlice->Refresh, *this, &VolumeRayCaster::onRefreshBaseSliceChannel );    
            myImp->mySliceBaseListenerCache.insert(make_pair(channelSlice, make_pair(fenceSlate,true) )); 
            //onRefreshBaseSliceChannel(channelSlice);
        }
//        else
//        {
//            assert(false); 
//        }
    }
    if(activeVolSVO)
    {
        int volIdx = fenceSlate->getVolumeIndex(*(activeVolSVO));
        SliceBase* channelSlice = fenceSlate->getSlice(volIdx); 
        if(myImp->mySliceBaseListenerCache.count(channelSlice) == 0)
        { 
            Connect( *channelSlice, channelSlice->Refresh, *this, &VolumeRayCaster::onRefreshBaseSliceChannel );    
            myImp->mySliceBaseListenerCache.insert(make_pair(channelSlice, make_pair(fenceSlate,true))); 
            //onRefreshBaseSliceChannel(channelSlice);
        }
//        else
//        {
//            assert(false); 
//        }
    }
}


void VolumeRayCaster::onPreDeleteFenceSlate(SlateBase* fenceSlate)
{
    int id = myImp->myVSM->getFenceIndex(*fenceSlate);
    int fenceSetId = myImp->myVSM->getFenceSet(*fenceSlate);
    if(id >= 0 )
    {
        myImp->myVolumeRayCasterAlgo->deleteFenceSliceNode(fenceSetId); 
    }
    
    for(int i = 0; i < fenceSlate->getSlicesNum(); i++)
    {
        SliceBase* slc = fenceSlate-> getSlice(i); 
        if( myImp->mySliceBaseListenerCache.count(slc) > 0 )
        {
            Disconnect( *slc, slc->Refresh, *this, &VolumeRayCaster::onRefreshBaseSliceChannel );     
            myImp->mySliceBaseListenerCache.erase(slc); 
        }    
    }
}


void VolumeRayCaster::onPostCurFenceSetChanged(VolumeSlateManager* vsm)
{
    myImp->myVolumeRayCasterAlgo->updateCurrentFenceSet(vsm->getCurFenceSet()); 
}

void VolumeRayCaster::onPostAddCurvedSlate(CurvedSlate* curvSlt)
{
    //connect signal, and cache it
    Volume* activeVolSVO = NULL; 
    Volume* extraTorVol = NULL; 
    myImp->getSVOVolumeState(activeVolSVO, extraTorVol);
    
    char sliceName[128]; 
    int curvSlateId = myImp->myVSM->getCurvedSlateIndex(curvSlt); 
    vector<vlSceneGraphNode*> nodes;
    sprintf(sliceName, "CurvedFence_%d", curvSlateId);
    vlSceneGraphManager::getInstance()-> getSceneGraphNodesByName(vString(sliceName), nodes); 
    if(nodes.size() == 0 )
    {            
        vlSceneGraphNode* newNode = new vlSceneGraphNode(vString(sliceName)); 
        vlSceneGraphManager::getInstance()-> getRootNode()->addChildNode(newNode); 
        CurvedSliceRenderable* curveSliceRenderer = new CurvedSliceRenderable(); 
        newNode->addComponent(curveSliceRenderer);     
    }
    
    if(extraTorVol)
    {
        int volIdx = curvSlt->getVolumeIndex(*(extraTorVol));
        CurvedSlice* channelSlice = curvSlt->getCurvedSlice(volIdx);
        if(myImp->myCurvedSliceListenerCache.count(channelSlice) == 0)
        { 
            Connect( *channelSlice, channelSlice->Refresh, *this, &VolumeRayCaster::onRefreshCurvedSliceChannel );    
            myImp->myCurvedSliceListenerCache.insert(make_pair(channelSlice, make_pair(curvSlt,false) )); 
            //onRefreshCurvedSliceChannel(channelSlice); 
        }
        else
        {
            assert(false); 
        }
    }
    if(activeVolSVO)
    {
        int volIdx = curvSlt->getVolumeIndex(*(activeVolSVO));
        CurvedSlice* channelSlice = curvSlt->getCurvedSlice(volIdx); 
        if(myImp->myCurvedSliceListenerCache.count(channelSlice) == 0)
        { 
            Connect( *channelSlice, channelSlice->Refresh, *this, &VolumeRayCaster::onRefreshCurvedSliceChannel );    
            myImp->myCurvedSliceListenerCache.insert(make_pair(channelSlice, make_pair(curvSlt, false) )); 
           // onRefreshCurvedSliceChannel(channelSlice);
        }
        else
        {
            assert(false); 
        }
    }
}

void VolumeRayCaster::onPreDeleteCurvedSlate(CurvedSlate* curvSlt)
{
    int id = myImp->myVSM->getCurvedSlateIndex(curvSlt);
    if(id >= 0 )
    {
        myImp->myVolumeRayCasterAlgo->deleteCurvedFenceSliceNode(id); 
    }
    
    for(int i = 0; i < curvSlt->getNumVolumes(); i++)
    {
        CurvedSlice* slc = curvSlt->getCurvedSlice(i); 
        if( myImp->myCurvedSliceListenerCache.count(slc) > 0 )
        {
            Disconnect( *slc, slc->Refresh, *this, &VolumeRayCaster::onRefreshCurvedSliceChannel );     
            myImp->myCurvedSliceListenerCache.erase(slc); 
        }    
    }
}

void VolumeRayCaster::updateSceneGraph()
{
     myImp->updateSceneGraph(); 
}

void VolumeRayCaster::showSeismic(bool visible)
{
    myImp->myVolumeRayCasterAlgo->setAllShowSeismic(visible); 
}    

void VolumeRayCaster::showAttribute(bool visible)
{
    myImp->myVolumeRayCasterAlgo->setAllShowAttribute(visible);
}

void VolumeRayCaster::onHorizonGroupUnload(vgHorizonGroup* hg)
{
    //clean horizon clipping texture if needed
    if(hg->getHorizonIndex(myImp->myClippingHrz[0])!= -1)
    {
        myImp->myClippingHrz[0] = NULL; 
        GLuint clearColor = 0;
        glBindTexture(GL_TEXTURE_2D, myImp->myVolumeRayCasterAlgo->getClippingHrzTex2D(0));
        vlGLExt::glClearTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &clearColor);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    if(hg->getHorizonIndex(myImp->myClippingHrz[1])!= -1)
    {
        myImp->myClippingHrz[1] = NULL; 
        GLuint clearColor = 0;
        glBindTexture(GL_TEXTURE_2D, myImp->myVolumeRayCasterAlgo->getClippingHrzTex2D(1));
        vlGLExt::glClearTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &clearColor);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void VolumeRayCaster::onCurrentHorizonChange( vgHorizon* hrz )
{
//    if(!hrz)
//    {
//        myImp->myVolRenderConfig->setHorizonClip(false);
//        GLuint clearColor = 0;
//        glBindTexture(GL_TEXTURE_2D, myImp->myVolumeRayCasterAlgo->getClippingHrzTex2D());
//        vlGLExt::glClearTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &clearColor);
//        glBindTexture(GL_TEXTURE_2D, 0);
//    }
//    else
//    {
//        bool useHrzClip = myImp->myVolRenderConfig->isHorizonClip();
//        setClippingHrz(hrz); 
//        myImp->myVolRenderConfig->setHorizonClip(useHrzClip);
//    }
}

vlVolumeRenderingQuality& VolumeRayCaster::getVolRendQuality()
{
    return myImp->myVolumeRayCasterAlgo->getVolRendQuality(); 
}

//Seismic, Attribute and CoRender
void VolumeRayCaster::setMultiRenderingMode(size_t mode)
{
    myImp->myVolRenderConfig -> setMultipleVolumeRenderingMode(mode); 
    
    if(mode == VOLUMERENDER_NORENDER)
        return ; 
    
    static queue<size_t> historyRenderMode; 
    if( historyRenderMode.size() > 0)
    {
        size_t lastRenderMode = historyRenderMode.back(); 
        if(mode != lastRenderMode)
        {
            myImp -> myVolumeRayCasterAlgo->resetRenderState(); 
            myImp ->myVolumeRayCasterAlgo->setEnvelopeDirty(true); 
        }
        if(historyRenderMode.size()>10)
            historyRenderMode.pop(); 
        historyRenderMode.push(mode);
    }
    else
    {
        myImp->myVolRenderConfig -> setMultipleVolumeRenderingMode(mode); 
        myImp -> myVolumeRayCasterAlgo->resetRenderState(); 
        myImp ->myVolumeRayCasterAlgo->setEnvelopeDirty(true); 
        historyRenderMode.push(mode);
    }
}

size_t VolumeRayCaster::getMultiRenderingMode() const
{
    return myImp ->myVolRenderConfig->getMultipleVolumeRenderingMode(); 
}

//Interactive, Auto Or Quality
void VolumeRayCaster::setInteractiveMode(size_t mode) 
{
    if( myImp -> myVolRenderConfig -> getInteractiveMode() != (INTERACTIVE_MODE)mode )
    {
        myImp -> myVolRenderConfig -> setInteractiveMode( (INTERACTIVE_MODE)mode ); 
        
        myImp ->myVolumeRayCasterAlgo->resetRenderState(); 
    }  
}

size_t VolumeRayCaster::getInteractiveMode( ) const
{
    return myImp -> myVolRenderConfig ->getInteractiveMode() ; 
}

void VolumeRayCaster::setVisHorizonImage(bool flag)
{
    myImp ->myVisualizeHrzImage = flag; 
}

bool VolumeRayCaster::isVisHorizonImage() const
{
    return myImp ->myVisualizeHrzImage ;
}
    
void VolumeRayCaster::setVisSSAOImage(bool flag)
{
    myImp->myVisualizeSSAOImage = flag; 
}

bool VolumeRayCaster::isVisSSAOImage() const
{
    return myImp->myVisualizeSSAOImage; 
}
    
void VolumeRayCaster::setVisSSAOBlurredImage(bool flag)
{
    myImp->myVisualizeSSAOBlurredImage = flag; 
}

bool VolumeRayCaster::isVisSSAOBlurredImage() const
{
    return myImp->myVisualizeSSAOBlurredImage; 
}

void VolumeRayCaster::setBlurSSAO(bool flag)
{
    myImp->myVolumeRayCasterAlgo->setSSAOUseBlurredPass(flag); 
}

bool VolumeRayCaster::isBlurSSAO() const
{
    return myImp->myVolumeRayCasterAlgo->isSSAOUseBlurredPass(); 
}

bool VolumeRayCaster::setClippingHrz(int hid, vgHorizon* horizon)
{
    //hid: 0 top salt, 
    //hid: 1 bottom salt
    //Now only hard coded "Depth" horizon attribute. Need to Do
    while(vlGLExt::checkGLErrors("before setClippingHrz"))
    {
        continue;
    }

    if(myImp->myClippingHrz[hid] != horizon )
    {
        if(horizon == NULL)
            return false; 
        
        Vect4i size_org_hrz = horizon->getSize();
        Vect4d upper_hrz = horizon ->getUpper(); 
        
        if( myImp->myClippingHrz[hid] == 0 || myImp->myClippingHrz[hid] ->getSize()  != size_org_hrz )
        {   
            glBindTexture( GL_TEXTURE_2D, myImp->myVolumeRayCasterAlgo->getClippingHrzTex2D(hid));
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ); 
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ); 
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 ); 
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
            
            //cautious: glTexStorage2D allocate once and cannot be changed immutable memory allocation. 
            glTexImage2D(GL_TEXTURE_2D, GLint(0), GL_R16F, size_org_hrz[0], size_org_hrz[1], 0, GL_RED,  GL_FLOAT, NULL );
            //vlGLExt::glTexStorage2D(GL_TEXTURE_2D, GLint(1), GL_R16F, size_org_hrz[0], size_org_hrz[1]);
            
            vlGLExt::checkGLErrors("after glTexStorage2D");
            glBindTexture(GL_TEXTURE_2D, 0); 
        }
        //Could be made available to other horzion attribute 
        float *z_org_hrz = horizon->getAttrPtr( ATTR_NAME_DEPTH );
        if( z_org_hrz == 0 ) 
        {
            return false;
        }
        
        float *z_hrz = new float[size_org_hrz[0] * size_org_hrz[1]];       
        for(int i = 0; i < size_org_hrz[0]; i++)
        {
            for(int j = 0; j < size_org_hrz[1]; j++)                
            {
                float zij = *(z_org_hrz + i * size_org_hrz[1] + j);
                if(isNoValue(zij))
                    *(z_org_hrz + i * size_org_hrz[1] + j) = 0;    //0: NOT CLIP
                *(z_hrz + j * size_org_hrz[0] + i) = zij; 
            }
        }
              
        int align; 
        glGetIntegerv( GL_UNPACK_ALIGNMENT, &align);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelTransferi( GL_MAP_COLOR, 0 ); // turn off in case this was on

        glBindTexture( GL_TEXTURE_2D, myImp->myVolumeRayCasterAlgo->getClippingHrzTex2D(hid));
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size_org_hrz[0], size_org_hrz[1], GL_RED, GL_FLOAT, z_hrz);
        glBindTexture(GL_TEXTURE_2D, 0);  
        glPixelStorei(GL_UNPACK_ALIGNMENT, align);
        glPixelTransferi( GL_MAP_COLOR, 1 ); // turn off in case this was on
        
        delete[] z_hrz; 
        myImp->myClippingHrz[hid] = horizon; 
    }
    
    return !vlGLExt::checkGLErrors("after setClippingHrz");
}

//Regular mode
bool VolumeRayCaster::checkMultiLDMCompatability( vString seisLDMPath, vString attrLDMPath, vString& errMsg ) 
{
    std::vector<vString> ldmPaths; 
    ldmPaths.push_back(seisLDMPath); 
    ldmPaths.push_back(attrLDMPath); 
    return OctreeUtils::checkLDMCompatablity(ldmPaths, errMsg);
}
//RGBA mode
bool VolumeRayCaster::checkMultiVolumeCompatability(Volume *seisVol, Volume* attrVol, vString& errMsg )
{
    std::vector<Volume*> torVolumes; 
    torVolumes.push_back(seisVol); 
    torVolumes.push_back(attrVol); 
    return vlSparseVolumeOctreeWrapper::checkTorVolumeCompatablity(torVolumes, errMsg);
}

bool VolumeRayCaster::checkMultiLDMCompatability( vector<vString> channelLDMPath, vString& errMsg)
{
    return OctreeUtils::checkLDMCompatablity(channelLDMPath, errMsg);
}

bool VolumeRayCaster::checkMultiVolumeCompatability(vector<Volume*> channelVols,  vString& errMsg )
{
    return vlSparseVolumeOctreeWrapper::checkTorVolumeCompatablity(channelVols, errMsg);
}
//Regular mode
void VolumeRayCaster::setVolumeInput( Volume *seisVol, Volume *attrVol, vString seisLDMPath, vString attrLDMPath,
        Spectrum& seisVolSpec, Spectrum& attrVolSpec, 
        bool fromLDM, 
        bool exportLDM, 
        const vString& outputDir)
{
    blockRendering = true; 
    blockUpdate = true; 
 
    //clean obsolete resource 
    myImp ->myVolumeRayCasterAlgo->invalidateNodeBuffer(); 
    myImp ->myVolumeRayCasterAlgo->invalidateBrickBuffer(); 
    myImp ->invalidateGridTexture(); 
    myImp ->invalidateGridBuffer();
    myImp ->myVolumeRayCasterAlgo ->invalidateStreamSliceTexture(); 
    myImp ->myVolumeRayCasterAlgo->resetRenderState(); 
    myImp ->invalidateTransferFunction(); 
    
    myImp->mySeisVol = seisVol;
    myImp->myAttrVol = attrVol; 
    myImp->mySeisVolSpec = seisVolSpec; 
    myImp->myAttrVolSpec = attrVolSpec; 
    myImp->mySeisLDMPath = seisLDMPath; 
    myImp->myAttrLDMPath = attrLDMPath; 
    
    myImp->myIsRGBAMode = false; 
    myImp->myVolumeRayCasterAlgo->setRGBAMode(false); 
    
    if( myImp->updateVolumeOctree(fromLDM) ) 
    {
#ifdef ENABLEOBJECTEMPTY
        myImp -> myVolumeRayCasterAlgo ->initializeGridDimension(); 
        {
           if( myImp->mySeisVol && !myImp->myAttrVol ) 
                myImp ->computeGridTexture(myImp->mySeisVol, NULL, myImp -> myVolumeRayCasterAlgo->getGridDim(),
                        myImp -> myVolumeRayCasterAlgo->getVoxelsPerCell() );
            else if( !myImp->mySeisVol && myImp->myAttrVol )
               myImp ->computeGridTexture(myImp->myAttrVol, NULL, myImp -> myVolumeRayCasterAlgo->getGridDim(),
                        myImp -> myVolumeRayCasterAlgo->getVoxelsPerCell());
            else if(myImp->mySeisVol && myImp->myAttrVol)
                myImp ->computeGridTexture(myImp->mySeisVol,myImp->myAttrVol, myImp -> myVolumeRayCasterAlgo->getGridDim(),
                        myImp -> myVolumeRayCasterAlgo->getVoxelsPerCell() );

            myImp ->myVolumeRayCasterAlgo->updateGridTexture(myImp->myGridTexture3D);
        }
        myImp ->myVolumeRayCasterAlgo->initializeGridBuffer(); 
        myImp ->myVolumeRayCasterAlgo->createProxyCube(); 
        myImp ->myVolumeRayCasterAlgo->setEnvelopeDirty(true);  
#endif
        if(exportLDM)
            this ->exportSVO2LDM( seisVol, attrVol, outputDir ) ;
        
        //update vram and stream bandwidth w.r.t volume size
        myImp->autoSetVRAMBudget(myImp->myOctreeVol->getOctreeDimension()[0]); 
    }
    else
    {
        myImp->mySeisVol = NULL;
        myImp->myAttrVol = NULL; 
        myImp->mySeisLDMPath.erase(); 
        myImp->myAttrLDMPath.erase(); 
    }
    
    // initialzed scene graph: existed slice geometry
    myImp->updateSceneGraph(); 
    blockRendering = false; 
    blockUpdate = false; 
}
//rgba mode, not implemented empty space skipping, doubt its necessaty 
// not implemented scene graph. 
void VolumeRayCaster::setRGBAVolumeInput( vector<Volume*>& channelVols, vector<vString> channelPath, bool fromLDM, 
                        bool exportLDM, const vString& outputDir)
{
    blockRendering = true; 
    blockUpdate = true; 
 
    myImp ->myVolumeRayCasterAlgo->invalidateNodeBuffer(); 
    myImp ->myVolumeRayCasterAlgo->invalidateBrickBuffer(); 
    myImp ->invalidateGridTexture(); 
    myImp ->invalidateGridBuffer();
    myImp ->myVolumeRayCasterAlgo ->invalidateStreamSliceTexture(); 
    myImp ->myVolumeRayCasterAlgo->resetRenderState(); 
    myImp ->invalidateTransferFunction(); 
    
    myImp->myChannelVols = channelVols; 
    myImp->myChannelLDMPaths = channelPath; 
    
    myImp->myIsRGBAMode = true; 
    myImp->myVolumeRayCasterAlgo->setRGBAMode(true); 
    
    if( myImp->updateVolumeOctreeRGBA(fromLDM) ) 
    {
#ifdef ENABLEOBJECTEMPTY
//
//        myImp -> myVolumeRayCasterAlgo ->initializeGridDimension(); 
//        {
//           if( myImp->mySeisVol && !myImp->myAttrVol ) 
//                myImp ->computeGridTexture(myImp->mySeisVol, NULL, myImp -> myVolumeRayCasterAlgo->getGridDim(),
//                        myImp -> myVolumeRayCasterAlgo->getVoxelsPerCell() );
//            else if( !myImp->mySeisVol && myImp->myAttrVol )
//               myImp ->computeGridTexture(myImp->myAttrVol, NULL, myImp -> myVolumeRayCasterAlgo->getGridDim(),
//                        myImp -> myVolumeRayCasterAlgo->getVoxelsPerCell());
//            else if(myImp->mySeisVol && myImp->myAttrVol)
//                myImp ->computeGridTexture(myImp->mySeisVol,myImp->myAttrVol, myImp -> myVolumeRayCasterAlgo->getGridDim(),
//                        myImp -> myVolumeRayCasterAlgo->getVoxelsPerCell() );
//
//            myImp ->myVolumeRayCasterAlgo->updateGridTexture(myImp->myGridTexture3D);
//        }
        myImp ->myVolumeRayCasterAlgo->initializeGridBuffer(); 
        myImp ->myVolumeRayCasterAlgo->createProxyCube(); 
        myImp ->myVolumeRayCasterAlgo->setEnvelopeDirty(true);  
#endif
        if(exportLDM)
            this ->exportSVO2LDM(  myImp->myChannelVols , outputDir ) ;
        
        //update vram and stream bandwidth w.r.t volume size
        myImp->autoSetVRAMBudget(myImp->myOctreeVol->getOctreeDimension()[0]); 
    }
    else
    {
        myImp->myChannelVols.resize(0); 
        myImp->myChannelLDMPaths.resize(0); 
    }
    
    //
   // myImp->updateSceneGraph(); 
    blockRendering = false; 
    blockUpdate = false; 
}

void VolumeRayCaster::tryDeleteVolume( Volume* delVol)
{
    if(!blockDelete)
    {
        blockRendering = true; 
        blockUpdate = true; 
        if(myImp->mySeisVol == delVol || myImp->myAttrVol == delVol)
        {
            if(myImp->myOctreeVol)
            {
                vlSparseVolumeOctreeWrapper* tmpOctreeVol = dynamic_cast<vlSparseVolumeOctreeWrapper*>(myImp->myOctreeVol);

                if(tmpOctreeVol && tmpOctreeVol ->hasTornadoVol(delVol))
                {
                    delete myImp->myOctreeVol; 
                    myImp->myOctreeVol = 0; 
                    
                    myImp->myVolumeRayCasterAlgo->setCurrentOctreeVol(NULL);
                    myImp->myVolumeRayCasterAlgo->invalidateNodeBuffer(); 
                    myImp->myVolumeRayCasterAlgo->invalidateBrickBuffer(); 
                }
            }
            if(myImp->mySeisVol == delVol)
            {
                myImp->mySeisVol = 0; 
            }
            if(myImp->myAttrVol == delVol)
            {
                myImp->myAttrVol = 0; 
            }       
        }  
        blockRendering = false; 
        blockUpdate = false; 
    }
}

//test: create volume from svo 
Volume* VolumeRayCaster::createFdm(bool seismicSVO, Vect3i minRange, Vect3i maxRange)
{
    if( !myImp->myOctreeVol )
    {
        return NULL; 
    }
    
    int volId; 
    if(seismicSVO)
    {
        volId = 0; 
    }
    else
    {
        if(myImp ->mySeisVol && myImp ->myAttrVol)
            volId = 1; 
        else
            volId = 0; 
    }
    
    vlSparseVolumeOctreeWrapper* tmpOctreeVol = dynamic_cast<vlSparseVolumeOctreeWrapper*>(myImp->myOctreeVol);
    if(tmpOctreeVol)
    {
        return tmpOctreeVol ->createTornadoVolume(volId, seismicSVO ? myImp -> mySeisVol : myImp -> myAttrVol, "test", 0, minRange, maxRange ) ;
    }
    else
        return NULL; 
}

void VolumeRayCaster::resetRenderState()
{
    this->myImp->myVolumeRayCasterAlgo->resetRenderState(); 
}

void VolumeRayCaster::invalidateEnvelope()
{
    this->myImp->myVolumeRayCasterAlgo->setEnvelopeDirty(true); 
}

void VolumeRayCaster::setWellLogManager(WellLogManager* wlManager)
{
    this ->myImp->myWellLogManager = wlManager; 
}

void VolumeRayCaster::setPerformanceOutput(bool flag)
{
    this->myImp->myVolumeRayCasterAlgo->setPerformanceOutput(flag); 
}

bool VolumeRayCaster::isPerformanceOutput() const
{
    return myImp->myVolumeRayCasterAlgo->isPerformanceOutput(); 
}

//test purpose: create slice from svo
float* VolumeRayCaster::createSlice(size_t volId, SLICE_ALIGNMENT sliceAlign, int sliceNum, Vect3i minRange, Vect3i maxRange)
{
    if( !myImp->myOctreeVol )
    {
        return NULL; 
    }
    
    int volNum = myImp ->myOctreeVol ->getNumVolumes(); 
    if(volId >= volNum)
    {
        return NULL; 
    }
    vlSparseVolumeOctreeWrapper* tmpOctreeVol = dynamic_cast<vlSparseVolumeOctreeWrapper*>(myImp->myOctreeVol);
    if(tmpOctreeVol)
    {
        return tmpOctreeVol ->createTornadoSliceTexture(volId, sliceAlign, sliceNum, 0, minRange, maxRange); 
    }
    else
    {
        return NULL; 
    }
}

void VolumeRayCaster::updateExtraTorSpectrum(const Spectrum& torSpec, bool seismic)
{
    if(seismic)
        this->myImp->myCurrSeisVolSpec = torSpec; 
    else
        this->myImp->myCurrAttrVolSpec = torSpec; 
}

void VolumeRayCaster::syncTornadoSpectrumMem(Volume* torVol, Spectrum* torSpec)
{ 
    //each time spectrum setting is changed. 
    //1. invalidate transfunction texture 1d; 
    //2. need to recalculate tight entry/exit bounding box from empty space skipping step by call setEnvelopeDirty( true )
    myImp->invalidateExtraVolTransferFunction(); 
    if(torSpec-> getType() == DATA_SEISMIC)    
    {
        myImp->myCurrSeisVolSpec = *torSpec; 
    }
    
    if(torSpec-> getType() == DATA_SEISMIC)    
    {
        myImp->myCurrAttrVolSpec = *torSpec; 
    }    
    
    if(!myImp -> myOctreeVol)
        return ; 
    
    OctreeUtils::VOXFORMAT volumeType = OctreeUtils::UNKNOWN; 
    
    volumeType = vlSparseVolumeOctreeWrapper::getTornadoVolType(torVol);    
    int volChannelId = myImp->getTornadoVolumeChannel(torVol);
    
    if(volChannelId < 0 )
        return; 
    
    assert(volumeType == OctreeUtils::FDM || volumeType == OctreeUtils::VG || volumeType == OctreeUtils::CAPISINGLE);
    Spectrum* spectrum = const_cast<Spectrum*> (torSpec);
    
    Vect2d spec_range = torSpec->getRange();
    Vect2d data_range = torSpec -> getDataRange ();
      
    if(volChannelId == 1)
    {
        myImp->myVolumeRayCasterAlgo->setAttrVolDataTFRange(Vect2f( spec_range[0], spec_range[1] ), 
                Vect2f(data_range[0], data_range[1]) );
    }
    else
    {
        myImp->myVolumeRayCasterAlgo->setSeisVolDataTFRange(Vect2f( spec_range[0], spec_range[1] ), 
                Vect2f(data_range[0], data_range[1]));
    }

    float* color1d = new float[NUM_GRADE * 4]; 
    float* opacitySAT = new float[NUM_GRADE];
    myImp->spectrum2array(*spectrum, color1d, opacitySAT); 

    GLint align; 
    glGetIntegerv( GL_UNPACK_ALIGNMENT, &align);
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
    glPixelTransferi( GL_MAP_COLOR, 0 ); // turn off in case this was on
    
    glBindTexture( GL_TEXTURE_1D_ARRAY,  myImp->myVolumeRayCasterAlgo->getTransferFunctionTextureArray() ); 
    glTexSubImage2D(GL_TEXTURE_1D_ARRAY, GLint(0),  0, volChannelId, NUM_GRADE, 1, GL_RGBA, GL_FLOAT,color1d); 
    glBindTexture(GL_TEXTURE_1D_ARRAY, 0);

    glBindTexture( GL_TEXTURE_1D_ARRAY,  myImp->myVolumeRayCasterAlgo->getOpacitySATArray() ); 
    glTexSubImage2D(GL_TEXTURE_1D_ARRAY, GLint(0),  0, volChannelId, NUM_GRADE, 1, GL_RED, GL_FLOAT,opacitySAT); 
    glBindTexture(GL_TEXTURE_1D_ARRAY, 0);
    
    glPixelTransferi( GL_MAP_COLOR, 1 ); 
    glPixelStorei (GL_UNPACK_ALIGNMENT, align);
    
    delete[] color1d; 
    delete[] opacitySAT; 
    
    myImp ->myVolumeRayCasterAlgo->setEnvelopeDirty( true ) ; 
    myImp ->myVolumeRayCasterAlgo->resetRenderState(); 
}

void VolumeRayCasterImp::updateROI()
{
    vlAppModel& model = vlAppModel::getRef();
    
    vmGroupParams gp;
    model.getVolumeRenderingROIManip()->getGroupParams(model.getRefGeom(), gp);

    Vect4i size = model.getRefGeom().getSize(); 
    Vect4d origin = model.getRefGeom().getOrigin();        
    Vect4i gmin = model.getRefGeom().getMin();
    Vect4i ginc = model.getRefGeom().getInc();

    Vect4i rmin = model.getVolumeRenderingROIManip()->getMin(); 
    Vect4i rmax = model.getVolumeRenderingROIManip()->getMax(); 

    float slMinNormf = (float)( rmin[0] - gmin[0] ) / ginc[0];
    slMinNormf/= size[0]; 

    float slMaxNormf = (float)( rmax[0] - gmin[0] ) / ginc[0]; 
    slMaxNormf/= size[0]; 

    float xlMinNormf = (float)( rmin[1] - gmin[1] ) / ginc[1]; 
    xlMinNormf /= size[1]; 

    float xlMaxNormf = (float)( rmax[1] - gmin[1] ) / ginc[1];     
    xlMaxNormf /= size[1]; 

    float zMinNormf =  (float)(rmin[2] - gmin[2] ) / ginc[2]; 
    zMinNormf /= size[2]; 
    float zMaxNormf =  (float)(rmax[2] - gmin[2] ) / ginc[2]; 
    zMaxNormf /= size[2]; 
    
    myVolumeRayCasterAlgo->updateBoundingBoxRange( Vect3f(slMinNormf,xlMinNormf, zMinNormf ),  Vect3f(slMaxNormf,xlMaxNormf,zMaxNormf ) );   
}

//this is default settings for vram and streaming budget
void VolumeRayCasterImp::autoSetVRAMBudget(size_t octreeDim)
{
    switch(octreeDim)
    {
        case 8192:
            this->myInstance->setVRAMCacheBudgetMB(4096, 256); 
            break;
        case 4096:
            this->myInstance->setVRAMCacheBudgetMB(2048, 128); 
        case 2048:
            this->myInstance->setVRAMCacheBudgetMB(1024, 128);     
            break;
        case 1024:
            this->myInstance->setVRAMCacheBudgetMB(512, 64);     
            break;  
        case 512:
            this->myInstance->setVRAMCacheBudgetMB(512, 64);     
            break;      
        default:
            this->myInstance->setVRAMCacheBudgetMB(256, 64);
            break; 
    }
}

//void VolumeRayCasterImp::drawLights()
//{
//    while(vlGLExt::checkGLErrors("before drawLights"))
//    {
//        continue;
//    }   
//    
//    vlGLExt::glBindFramebufferEXT( GL_FRAMEBUFFER, 0 ); 
//    
//    Matrixf viewprojectionMatrix = this->myVolumeRayCasterAlgo->getViewProjectionMatrix(); 
//    Matrixf worldMatrix; 
//    worldMatrix.settrans(Vect3f(0.4, 0.5, 0.9) );
//    worldMatrix.mat[0] =  worldMatrix.mat[5] =  worldMatrix.mat[10] = 0.06; 
//
//    Matrixf worldviewprojectionMatrix = viewprojectionMatrix * worldMatrix; 
//    
//    getDrawObjectTechnique()->use(); 
//    getDrawObjectTechnique()->setMVPMatrixf((float*)&worldviewprojectionMatrix.mat);
//    getDrawObjectTechnique()->setWorldMatrixf((float*)&worldMatrix.mat);
//    getDrawObjectTechnique()->release();
//    
//    getDrawObjectTechnique()->bindPipeline(); 
//    this->myLightSphere->draw(); 
//    getDrawObjectTechnique()->unbindPipeline();  
//    
//    vlGLExt::checkGLErrors("after drawLights");
//}
//
//vlDrawObjectTechnique* VolumeRayCasterImp::getDrawObjectTechnique()
//{
//    GLContext* ctx = GLContext::getCurrent();
//    if(this->myDrawObjectTechnique == NULL)
//    {
//        myDrawObjectTechnique = new vlDrawObjectTechnique(); 
//        if(!myDrawObjectTechnique ->init(ctx, GLSLProgramFactory::eDrawObject, 
//                GL_VERTEX_SHADER_BIT|GL_FRAGMENT_SHADER_BIT, false, NULL, false))
//            fprintf(stderr, "error init the myDrawObjectTechnique technique\n"); 
//    }
//    
//    return myDrawObjectTechnique;
//}

void VolumeRayCaster::syncTornadoMatrix(timeval cur_t, VolumeSlateManager& vsm, const BaseGeometry& refGeom, Matrixd& concat, Matrixd& inverseconcat,
        double& width, double& height, double& depth, 
        bool xlvis, bool ilvis, bool zvis, 
        bool isShowSeis, bool isShowAttr )
{
    //1. get the ModelViewProjection Matrix
    //2. refresh slice geometry, streaming geometry/texture to svo
    //3. pass some geometry down to svo 3rd party
    
    if(!myImp->myOctreeVol)
        return ; 
    
    if(blockUpdate)
    {
        return ;
    }
    blockDelete = true; 
    
    this->myImp->updateROI(); 
    
    Matrixf newMvpMat, newMvMat, newPrjMat, newImvpMat, newImvMat, newIprjMat; 
#ifdef USE_DOUBLEPRECISION
    Matrixd newMVPMatd; 
#endif
    
    Matrixf concatf; 
    Matrixf inverseconcatf; 
    for( int i = 0; i < 16; i++){
        concatf[i] = (float)concat[i]; 
        inverseconcatf[i] = (float)inverseconcat[i]; 
    }
    Vect3i size; 
    Vect3d origin(0, 0, 0), step( 1, 1, 1), extent( 1, 1, 1); 
    size = refGeom.getSize(); 
    
    origin = refGeom.getOrigin(); 
    step = refGeom.getStep(); 
    
    extent[0] = step[0] * (size[0] - 1);
    extent[1] = step[1] * (size[1] - 1);
    extent[2] = step[2] * (size[2] - 1);
    
    Matrixf translation_mat, scale_mat;
    translation_mat.identity();
    scale_mat.identity();
    
    Matrixd translation_matd, scale_matd;
    translation_matd.identity();
    scale_matd.identity();

    translation_mat.mat[12] = origin[0];
    translation_mat.mat[13] = origin[1];
    translation_mat.mat[14] = origin[2];

    //cross line slice 
    scale_mat.mat[0] = extent[0];
    scale_mat.mat[5] = extent[1];
    scale_mat.mat[10] = extent[2]; 
    
    translation_matd.mat[12] = origin[0];
    translation_matd.mat[13] = origin[1];
    translation_matd.mat[14] = origin[2];

    //cross line slice 
    scale_matd.mat[0] = extent[0];
    scale_matd.mat[5] = extent[1];
    scale_matd.mat[10] = extent[2]; 
    
    //something related to coordinates 
    Matrixd baseGeom_lr_d;
    //baseGeom_wr_d;
    Matrixf baseGeom_lr_f;
    //baseGeom_wr_f;  
    Matrixf baseGeom_lr_inverse_f; 
   
    Matrixd m = refGeom.calcMatrix( BaseGeometry::LOGICAL,BaseGeometry::RENDERING );
    
    Matrixf modelview; 
    Matrixf inverseModelView; 
    Matrixf worldMat;  
    Matrixd modelviewd; 
    
    modelview.identity(); 
    inverseModelView.identity(); 
    worldMat.identity();
    
    modelviewd.identity(); 
    
    Matrixf inversetranslation_mat = translation_mat; inversetranslation_mat.invert(); 
    Matrixf inversescale_mat = scale_mat; 
    inversescale_mat.mat[0] = 1.0f/  scale_mat.mat[0]; 
    inversescale_mat.mat[5] = 1.0f/  scale_mat.mat[5]; 
    inversescale_mat.mat[10] = 1.0f/  scale_mat.mat[10]; 
    
    modelview = modelview * baseGeom_lr_f;
    modelview = modelview * concatf; 
    modelview = modelview * translation_mat;
    modelview = modelview * scale_mat;
    
    modelviewd = modelviewd * concat; 
    modelviewd = modelviewd * translation_matd;
    modelviewd = modelviewd * scale_matd; 
    modelviewd = modelviewd * baseGeom_lr_d; 
    
    worldMat = worldMat * translation_mat;
    worldMat = worldMat * scale_mat;
    
    inverseModelView = inverseModelView * baseGeom_lr_inverse_f; 
    inverseModelView = inverseModelView * inversescale_mat;
    inverseModelView = inverseModelView * inversetranslation_mat; 
    inverseModelView = inverseModelView * inverseconcatf; 
    
    double r, l, t, b, n, f;
    r = width * 0.5;
    l = -r;
    t = height * 0.5;
    b = -t;
    f = depth * 0.5;
    n = -f;

    Matrixf projMat; 
    Matrixd projMatd; 
    Matrixf projMatInverse; 
    projMat[0] = (float)1/r; 
    projMat[5] = (float)1/t; 
    projMat[10] = (float)(-2/( f - n ) );
    
    projMatd[0] = (double)1/r; 
    projMatd[5] = (double)1/t; 
    projMatd[10] = (double)(-2/( f - n ) );

    projMatInverse = projMat; 
    projMatInverse[0] = r; 
    projMatInverse[5] = t;
    projMatInverse[10] = (f - n) / (-2.0f);
    
    newMvpMat = projMat * modelview ;
#ifdef USE_DOUBLEPRECISION
    newMvpMatd = projMatd * modelviewd; 
#endif
    newImvpMat = inverseModelView * projMatInverse;
    newImvMat = modelview ; 
    newImvMat.invert(); 
    newIprjMat = projMatInverse;
    newMvMat = modelview; 
    newPrjMat = projMat; 
    
    myImp->myVolumeRayCasterAlgo->updateMatrix(cur_t, newMvpMat,newMvMat, newPrjMat, 
                newImvpMat, newImvMat, newIprjMat, width, height, depth); 
    
    Vect3f eyePos(inverseModelView.mat[12], inverseModelView.mat[13], inverseModelView.mat[14] ); 
    eyePos /= inverseModelView.mat[15];
    Vect3f lightPos = eyePos + Vect3f(0,0,1); 
    float* lightDir = myImp->myTornadoView->getLight( GL_LIGHT0 ).getDirection();
    if(lightDir)
        lightPos = eyePos + Vect3f(lightDir[0], lightDir[1], lightDir[2]); 
     
    myImp->myVolumeRayCasterAlgo->updateCameraEye(Vect3f( eyePos[0], eyePos[1], eyePos[2] ), lightPos); 
   
    {
        //basic slate
        Vect3f newCurrentSliceCube; //slice num , xl, il, z, current reference geometry
        Vect3i newSliceVis; // slice vis, xl, il, z
        Vect2i newShowSeisAttr;  // state of show seis, show attribute
        Vect3i newCurrentExtraSliceNum; // extra slice, 
        Vect3i newUpdateTextureExtraSliceNum(1,1,1); 
        Vect3i newExtraTorSliceValid(1,1,1); 
        Vect3i newCurrentSliceNum; 
        Vect3i newUpdateTextureSliceNum(1,1,1); 
        Vect3i newVolRendSliceValid(1,1,1);
        Vect3i newCurrentRefGeomSize; 
          
        Vect4d lower = vsm.getLower();
        Vect4d upper = vsm.getUpper();

        SlateBase* sltcross, *sltinline, *sltz; 
        Vect2d dimcross, diminline, dimz;
        Matrixd matcross, matinline, matz;

        //local
        Vect4d lbgncross, lendcross;
        Vect4d lbgninline, lendinline;
        Vect4d lbgnz, lendz;
        //cube
        Vect4d cbgncross, cendcross;
        Vect4d cbgninline, cendinline;
        Vect4d cbgnz, cendz;

        int axis_hcross, axis_hinline, axis_hz;
        Vect2i slc_size_cross, slc_size_inline, slc_size_z;

        Matrixd l2g;
        l2g = refGeom.calcMatrix(BaseGeometry::LOGICAL, BaseGeometry::GRID);
            
        sltcross = vsm.getBasicSlate(0);
        sltinline = vsm.getBasicSlate(1);
        sltz = vsm.getBasicSlate(2);
        
        if (sltcross) {
            matcross = sltcross->getS2L();
            dimcross = sltcross->getDim();
            sltcross->getDefin(axis_hcross, lbgncross, lendcross);
            cbgncross = l2g * lbgncross; //survey to grid// voxel
            cendcross = l2g * lendcross;
            newCurrentSliceCube[0] = cbgncross[0]; 
            for (int i = 0; i < 3; ++i) {
                cbgncross[i] = int(cbgncross[i] + .5);
                cendcross[i] = int(cendcross[i] + .5);            
            }      
        }
        if (sltinline) {
            diminline = sltcross->getDim();
            matinline = sltinline->getS2L();
            sltinline->getDefin(axis_hinline, lbgninline, lendinline);
            cbgninline = l2g * lbgninline;
            cendinline = l2g * lendinline;
            newCurrentSliceCube[1] = cbgninline[1];
            for (int i = 0; i < 3; ++i) {
                cbgninline[i] = int(cbgninline[i] + .5);
                cendinline[i] = int(cendinline[i] + .5);
            }
        }
        if (sltz) {
            dimz = sltcross->getDim();
            matz = sltz->getS2L();
            sltz->getDefin(axis_hz, lbgnz, lendz);
            cbgnz = l2g * lbgnz;
            cendz = l2g * lendz;
            newCurrentSliceCube[2] = cbgnz[2]; 
            for (int i = 0; i < 3; ++i) {
                cbgnz[i] = int(cbgnz[i] + .5);
                cendz[i] = int(cendz[i] + .5);
            }     
        }      
    
        newSliceVis[1] = xlvis; 
        newSliceVis[0] = ilvis; 
        newSliceVis[2] = zvis; 
        
        newShowSeisAttr[0] = isShowSeis; 
        newShowSeisAttr[1] = isShowAttr; 
        
        //
        for( map< SliceBase*, pair<SlateBase*, bool> >::iterator itr = myImp->mySliceBaseListenerCache.begin(); itr != myImp->mySliceBaseListenerCache.end(); ++itr)
        {
            if(itr->second.second)
            {
               RefreshBaseSliceChannel(itr->first);                
               itr->second.second = false; 
            }
        }
        for( map<CurvedSlice*, pair<CurvedSlate*, bool> >::iterator itr = myImp->myCurvedSliceListenerCache.begin(); itr != myImp->myCurvedSliceListenerCache.end(); ++itr)
        {
            if(itr->second.second)
            {
               RefreshCurvedSliceChannel(itr->first);                
               itr->second.second = false; 
            }
        }

        BaseGeometry* geom = vlAppModel::getRef().getCurGeometry();
        if(geom )
        {
            myImp ->myVolumeRayCasterAlgo ->setReferenceGeometry(Vect3f( geom->getOrigin()[0], geom->getOrigin()[1], geom->getOrigin()[2]),
                    Vect3i( geom->getSize()[0], geom->getSize()[1], geom->getSize()[2]), 
            Vect3f( geom->getStep()[0], geom->getStep()[1], geom->getStep()[2]) ); 
        }
        
        if(!myImp->myIsRGBAMode)
        {
            Volume* activeVolume = myImp->mySeisVol ? myImp->mySeisVol : myImp->myAttrVol; 
            Volume* extraVolume = NULL;
            if( (myImp->mySeisVol  && myImp->myAttrVol) == 0)
            {
                if(activeVolume== myImp->mySeisVol)
                {
                    extraVolume = vlAppModel::getRef().getCurAttribute();
                }
                else
                {
                    extraVolume = vlAppModel::getRef().getCurSeismic();
                }
            }
          
            if(activeVolume)
            {
                myImp ->myVolumeRayCasterAlgo -> setActiveVolumeGeometry(Vect3f( activeVolume->getOrigin()[0], activeVolume->getOrigin()[1], activeVolume->getOrigin()[2]),
                Vect3i( activeVolume->getSize()[0], activeVolume->getSize()[1], activeVolume->getSize()[2]), 
                Vect3f( activeVolume->getStep()[0], activeVolume->getStep()[1], activeVolume->getStep()[2])); 
            }

            if(extraVolume)
            {
                myImp ->myVolumeRayCasterAlgo -> setExtraVolumeGeometry(Vect3f( extraVolume->getOrigin()[0], extraVolume->getOrigin()[1], extraVolume->getOrigin()[2]),
                Vect3i( extraVolume->getSize()[0], extraVolume->getSize()[1], extraVolume->getSize()[2]), 
                Vect3f( extraVolume->getStep()[0], extraVolume->getStep()[1], extraVolume->getStep()[2])); 
            }

            if(myImp->mySeisVol)
            {
                Vect4f linearDivergenceGainParameter; 
                vector<float> gain_params;
                myImp->mySeisVol -> getGainParam( LINEAR_GAIN, gain_params );
                linearDivergenceGainParameter[0] = gain_params[0]; 
                linearDivergenceGainParameter[1] = gain_params[1]; 

                myImp->mySeisVol -> getGainParam( DIVERGENCE_GAIN, gain_params );
                linearDivergenceGainParameter[2] = gain_params[0]; 
                linearDivergenceGainParameter[3] = gain_params[1]; 

                myImp ->myVolumeRayCasterAlgo ->setCurrentSeisGain((int)myImp->mySeisVol->getGainType(), 
                       linearDivergenceGainParameter, 
                        myImp->mySeisVol->getGain().getSRate(),
                        Vect2f(myImp->mySeisVol->getLower()[2], myImp->mySeisVol->getUpper()[2])); 
            }
        }
        else
        {
            if(myImp ->myChannelVols.size() > 0 && myImp ->myChannelVols[0])
            {
                Volume* activeVolume = myImp ->myChannelVols[0];
                myImp ->myVolumeRayCasterAlgo -> setActiveVolumeGeometry(Vect3f( activeVolume->getOrigin()[0], activeVolume->getOrigin()[1], activeVolume->getOrigin()[2]),
                Vect3i( activeVolume->getSize()[0], activeVolume->getSize()[1], activeVolume->getSize()[2]), 
                Vect3f( activeVolume->getStep()[0], activeVolume->getStep()[1], activeVolume->getStep()[2])); 
            }
        }
        
        //clipping horizon geometry 
        if(myImp->myClippingHrz[0])
        {
            myImp ->myVolumeRayCasterAlgo -> setCurrentHrzGeometry(  0,  
            Vect3f( myImp->myClippingHrz[0]->getOrigin()[0], myImp->myClippingHrz[0]->getOrigin()[1], myImp->myClippingHrz[0]->getOrigin()[2]),
            Vect3i( myImp->myClippingHrz[0]->getSize()[0], myImp->myClippingHrz[0]->getSize()[1], myImp->myClippingHrz[0]->getSize()[2]), 
            Vect3f( myImp->myClippingHrz[0]->getStep()[0], myImp->myClippingHrz[0]->getStep()[1], myImp->myClippingHrz[0]->getStep()[2])); 
        }
        if(myImp->myClippingHrz[1])
        {
            myImp ->myVolumeRayCasterAlgo -> setCurrentHrzGeometry(  1,  
            Vect3f( myImp->myClippingHrz[1]->getOrigin()[0], myImp->myClippingHrz[1]->getOrigin()[1], myImp->myClippingHrz[1]->getOrigin()[2]),
            Vect3i( myImp->myClippingHrz[1]->getSize()[0], myImp->myClippingHrz[1]->getSize()[1], myImp->myClippingHrz[1]->getSize()[2]), 
            Vect3f( myImp->myClippingHrz[1]->getStep()[0], myImp->myClippingHrz[1]->getStep()[1], myImp->myClippingHrz[1]->getStep()[2])); 
        }
    }
    
    blockDelete = false; 
}

void VolumeRayCaster::setSVOProgress( TaskStatus* progress )
{ 
    myImp -> mySVOBuildProgress = progress; 
} 

VolumeOctreeConfig* VolumeRayCaster::getVolOctreeConfig()
{
    return myImp ->  myVolOctreeConfig; 
}
VolumeRenderConfig* VolumeRayCaster::getVolRenderConfig()
{
   return myImp ->  myVolRenderConfig; 
}

void VolumeRayCaster::updateVolumeRenderingTech(int mode)
{   
    int curmode = myImp->myVolRenderConfig->getVolumeRenderingTech(); 
    if(curmode!= mode)
    {
        myImp->myVolRenderConfig->setVolumeRenderingTech(mode); 
        invalidateEnvelope();
    }
}

void VolumeRayCaster::updateIsoValue(Vect4f val)
{
    Vect4f curisoval = myImp->myVolRenderConfig->getIsoValue(); 
    if(curisoval != val)
    {
        myImp->myVolRenderConfig->setIsoValue(val); 
        //iso value will determine tight entry/exit boundary if empty space skipping is enabled
        invalidateEnvelope();
    }
}

void VolumeRayCaster::setVoxelPickLoc(Vect4d b)
{   
    Vect3i mouseLoc = myImp->myTornadoView->wnd2dev(
            myImp->myTornadoView->prj2wnd(
            myImp->myTornadoView->vwg2prj(
            myImp->myTornadoView->cab2vwg(
            myImp->myTornadoView->mod2cab(
            myImp->myTornadoView->phy2mod(
            myImp->myTornadoView->log2phy(b)))))));
    
    int width, height; 
    myImp->myTornadoView->getSize(width, height); 
    mouseLoc[1] = height - mouseLoc[1] ;

    myImp->myVolRenderConfig->setLastVoxelPickLoc(mouseLoc); 
}

void VolumeRayCaster::exportBinaryShader( )
{
    myImp->myVolumeRayCasterAlgo->exportBinaryShader(); 
}

void VolumeRayCaster::setVRAMCacheBudgetMB(int vramBudgetMB, int streamBudgetMB)
{
    int c_vramBudgetMB, c_streamBudgetMB; 
    myImp->myVolRenderConfig->getVRAMCacheBudgetMB(c_vramBudgetMB, c_streamBudgetMB);
    
    if(c_vramBudgetMB!= vramBudgetMB || streamBudgetMB!= c_streamBudgetMB)
    {
        //each time vram cache budget changed, needs to invalidate brick buffer 
        myImp->myVolRenderConfig->setVRAMCacheBudgetMB(vramBudgetMB, streamBudgetMB);
        myImp->myVolumeRayCasterAlgo->invalidateBrickBuffer(); 
    } 
}

void VolumeRayCaster::logVolRendProfile()
{
    myImp->myVolumeRayCasterAlgo->logVolRendProfile(); 
}

#else
VolumeRayCaster ::VolumeRayCaster(vmGLSmartView* torView): VolumeRayCasterDummy(torView)
{
    
}
#endif


VolumeRayCasterDummy :: VolumeRayCasterDummy( vmGLSmartView* torView, VolumeSlateManager* vsm)
{
    
}
    
VolumeRayCasterDummy::~VolumeRayCasterDummy()
{

}
   
void VolumeRayCasterDummy::setVolumeInput( Volume *seisVol, Volume* attrVol, vString seisLDMPath, vString attrLDMPath, 
                        Spectrum& seisVolSpect, Spectrum& attrVolSpect, bool fromLDM, bool exportLDM,const vString& outputDir )
{
    
}
    
void VolumeRayCasterDummy::tryDeleteVolume( Volume* delVol)
{
    
}

bool VolumeRayCasterDummy::checkMultiLDMCompatability(  vString seisLDMPath, vString attrLDMPath, vString& errMsg )
{
    return false; 
}

bool VolumeRayCasterDummy::checkMultiVolumeCompatability(Volume *seisVol, Volume* attrVol, vString& errMsg )
{
    return false; 
}

void VolumeRayCasterDummy::render(timeval t)
{
    
}

void VolumeRayCasterDummy::resize( int x, int y )
{
    
}

bool VolumeRayCasterDummy::isUseShaderRenderSlice() const
{
    return false; 
}

void VolumeRayCasterDummy::renderSlices()
{
    
}

void VolumeRayCasterDummy::syncTornadoMatrix(timeval t,  VolumeSlateManager& vsm, const BaseGeometry& refGeom, Matrixd& concat, Matrixd& inverseconcat,
        double& width, double& height, double& depth, bool xlvis, bool ilvis, bool zvis, bool isShowSeis, bool isShowAttr)
{
    
}

void VolumeRayCasterDummy::syncTornadoSpectrumMem(Volume* torVol, Spectrum* torSpec)
{
    
}
    
void VolumeRayCasterDummy::updateExtraTorSpectrum(const Spectrum& torSpec, bool seismic)
{
    
}
    
void VolumeRayCasterDummy::setMultiRenderingMode(size_t mode)
{
    
}

size_t VolumeRayCasterDummy::getMultiRenderingMode() const
{
    return LONG_MAX; 
}
   
void VolumeRayCasterDummy::setInteractiveMode(size_t mode)
{
    
}

size_t VolumeRayCasterDummy::getInteractiveMode( ) const
{
    return LONG_MAX; 
}
   
void VolumeRayCasterDummy::setSVOProgress( TaskStatus* progress )
{
    
}

bool VolumeRayCasterDummy::exportSVO2LDM(Volume *seisVol, Volume* attrVol, const vString& dir) 
{
    return false; 
}
    
vlVolumeRenderingQuality& VolumeRayCasterDummy::getVolRendQuality()
{
    static vlVolumeRenderingQuality vrq; 
    return vrq; 
}

void VolumeRayCasterDummy::deleteTriangleMeshGroup(TriangleMeshGroup *p)
{
    
}

void VolumeRayCasterDummy::newTriangleMeshGroup(TriangleMeshGroup *p)
{
    
}
    
bool VolumeRayCasterDummy::setClippingHrz(vgHorizon* horizon)
{
    return false; 
}
    
void VolumeRayCasterDummy::setVisHorizonImage(bool flag)
{
    
}
    
bool VolumeRayCasterDummy::isVisHorizonImage() const
{
    return false; 
}
    
Volume* VolumeRayCasterDummy::createFdm( bool seismicSVO, Vect3i minRange, Vect3i maxRange )
{
    return NULL; 
}

float* VolumeRayCasterDummy::createSlice(size_t volId, SLICE_ALIGNMENT sliceAlign, int sliceNum, Vect3i minRange, Vect3i maxRange)
{
    return NULL; 
}
    
void VolumeRayCasterDummy::setWellLogManager(WellLogManager* wlManager)
{
    
}
    
void VolumeRayCasterDummy::setPerformanceOutput(bool flag)
{
    
}

bool VolumeRayCasterDummy::isPerformanceOutput() const
{
    return false; 
}
    
VolumeOctreeConfig* VolumeRayCasterDummy::getVolOctreeConfig()
{
    return NULL; 
}

VolumeRenderConfig* VolumeRayCasterDummy::getVolRenderConfig()
{
    return NULL; 
}
    
void VolumeRayCasterDummy::updateVolumeRenderingTech(int mode)
{
    
}

void VolumeRayCasterDummy::updateIsoValue(Vect4f val)
{
    
}

void VolumeRayCasterDummy::setVoxelPickLoc(Vect4d b)
{
    
}

void VolumeRayCasterDummy::exportBinaryShader( )
{
    
}

void VolumeRayCasterDummy::setVRAMCacheBudgetMB(int vramBudgetMB, int streamBudgetMB)
{
    
}

void VolumeRayCasterDummy::logVolRendProfile()
{
    
}

void VolumeRayCasterDummy::onPostMakeRotatedSlate(SlateBase* traverseSlate)
{
    
}

void VolumeRayCasterDummy::onPreDeleteRotatedSlate(SlateBase* travseSlate)
{
    
}

void VolumeRayCasterDummy::onPostMakeFenceSlate(SlateBase* traverseSlate)
{
    
}

void VolumeRayCasterDummy::onPreDeleteFenceSlate(SlateBase* fenceSlate)
{
    
}

void VolumeRayCasterDummy::onPostCurFenceSetChanged(VolumeSlateManager* vsm)
{
    
}

void VolumeRayCasterDummy::onPostAddCurvedSlate(CurvedSlate* curvSlt)
{
    
}

void VolumeRayCasterDummy::onPreDeleteCurvedSlate(CurvedSlate* curvSlt)
{
    
}

void VolumeRayCasterDummy::updateSceneGraph()
{
    
}

void VolumeRayCasterDummy::showSeismic(bool visible)
{
    
}

void VolumeRayCasterDummy::showAttribute(bool visible)
{
    
}
