#include "DatasetUtils.h"

double DatasetUtils::CalcMetric_FMeasure(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN) {
    const double dRecall = DatasetUtils::CalcMetric_Recall(nTP,nTN,nFP,nFN);
    const double dPrecision = DatasetUtils::CalcMetric_Precision(nTP,nTN,nFP,nFN);
    return (2.0*(dRecall*dPrecision)/(dRecall+dPrecision));
}
double DatasetUtils::CalcMetric_Recall(uint64_t nTP, uint64_t /*nTN*/, uint64_t /*nFP*/, uint64_t nFN) {return ((double)nTP/(nTP+nFN));}
double DatasetUtils::CalcMetric_Precision(uint64_t nTP, uint64_t /*nTN*/, uint64_t nFP, uint64_t /*nFN*/) {return ((double)nTP/(nTP+nFP));}
double DatasetUtils::CalcMetric_Specificity(uint64_t /*nTP*/, uint64_t nTN, uint64_t nFP, uint64_t /*nFN*/) {return ((double)nTN/(nTN+nFP));}
double DatasetUtils::CalcMetric_FalsePositiveRate(uint64_t /*nTP*/, uint64_t nTN, uint64_t nFP, uint64_t /*nFN*/) {return ((double)nFP/(nFP+nTN));}
double DatasetUtils::CalcMetric_FalseNegativeRate(uint64_t nTP, uint64_t /*nTN*/, uint64_t /*nFP*/, uint64_t nFN) {return ((double)nFN/(nTP+nFN));}
double DatasetUtils::CalcMetric_PercentBadClassifs(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN) {return (100.0*(nFN+nFP)/(nTP+nFP+nFN+nTN));}
double DatasetUtils::CalcMetric_MatthewsCorrCoeff(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN) {return ((((double)nTP*nTN)-(nFP*nFN))/sqrt(((double)nTP+nFP)*(nTP+nFN)*(nTN+nFP)*(nTN+nFN)));}

cv::Mat DatasetUtils::ReadResult( const std::string& sResultsPath,
                                  const std::string& sCatName,
                                  const std::string& sSeqName,
                                  const std::string& sResultPrefix,
                                  size_t framenum,
                                  const std::string& sResultSuffix) {
    char buffer[10];
    sprintf(buffer,"%06lu",framenum);
    std::stringstream sResultFilePath;
    sResultFilePath << sResultsPath << sCatName << "/" << sSeqName << "/" << sResultPrefix << buffer << sResultSuffix;
    return cv::imread(sResultFilePath.str(),cv::IMREAD_GRAYSCALE);
}

void DatasetUtils::WriteResult( const std::string& sResultsPath,
                                const std::string& sCatName,
                                const std::string& sSeqName,
                                const std::string& sResultPrefix,
                                size_t framenum,
                                const std::string& sResultSuffix,
                                const cv::Mat& res,
                                const std::vector<int>& vnComprParams) {
    char buffer[10];
    sprintf(buffer,"%06lu",framenum);
    std::stringstream sResultFilePath;
    sResultFilePath << sResultsPath << sCatName << "/" << sSeqName << "/" << sResultPrefix << buffer << sResultSuffix;
    cv::imwrite(sResultFilePath.str(), res, vnComprParams);
}

void DatasetUtils::WriteOnImage(cv::Mat& oImg, const std::string& sText, bool bBottom) {
    cv::putText(oImg,sText,cv::Point(10,bBottom?(oImg.rows-15):15),cv::FONT_HERSHEY_PLAIN,1.0,cv::Scalar_<uchar>(0,0,255),1,CV_AA);
}

void DatasetUtils::WriteMetrics(const std::string sResultsFileName, const SequenceInfo* pSeq) {
    std::ofstream oMetricsOutput(sResultsFileName);
    MetricsCalculator tmp(pSeq);
    const std::string sCurrSeqName = pSeq->m_sName.size()>12?pSeq->m_sName.substr(0,12):pSeq->m_sName;
    std::cout << "\t\t" << std::setfill(' ') << std::setw(12) << sCurrSeqName << " : Rcl=" << std::fixed << std::setprecision(4) << tmp.m_oMetrics.dRecall << " Prc=" << tmp.m_oMetrics.dPrecision << " FM=" << tmp.m_oMetrics.dFMeasure << " MCC=" << tmp.m_oMetrics.dMCC << std::endl;
    oMetricsOutput << "Results for sequence '" << pSeq->m_sName << "' :" << std::endl;
    oMetricsOutput << std::endl;
    oMetricsOutput << "nTP nFP nFN nTN nSE" << std::endl; // order similar to the files saved by the CDNet analysis script
    oMetricsOutput << pSeq->nTP << " " << pSeq->nFP << " " << pSeq->nFN << " " << pSeq->nTN << " " << pSeq->nSE << std::endl;
    oMetricsOutput << std::endl << std::endl;
    oMetricsOutput << std::fixed << std::setprecision(8);
    oMetricsOutput << "Cumulative metrics :" << std::endl;
    oMetricsOutput << "Rcl        Spc        FPR        FNR        PBC        Prc        FM         MCC        " << std::endl;
    oMetricsOutput << tmp.m_oMetrics.dRecall << " " << tmp.m_oMetrics.dSpecficity << " " << tmp.m_oMetrics.dFPR << " " << tmp.m_oMetrics.dFNR << " " << tmp.m_oMetrics.dPBC << " " << tmp.m_oMetrics.dPrecision << " " << tmp.m_oMetrics.dFMeasure << " " << tmp.m_oMetrics.dMCC << std::endl;
    oMetricsOutput << std::endl << std::endl;
    oMetricsOutput << "Sequence FPS: " << pSeq->m_dAvgFPS << std::endl;
    oMetricsOutput.close();
}

void DatasetUtils::WriteMetrics(const std::string sResultsFileName, CategoryInfo* pCat) {
    std::ofstream oMetricsOutput(sResultsFileName);
    std::sort(pCat->m_vpSequences.begin(),pCat->m_vpSequences.end(),&SequenceInfo::compare);
    oMetricsOutput << "Results for category '" << pCat->m_sName << "' :" << std::endl;
    oMetricsOutput << std::endl;
    oMetricsOutput << "nTP nFP nFN nTN nSE" << std::endl; // order similar to the files saved by the CDNet analysis script
    oMetricsOutput << pCat->nTP << " " << pCat->nFP << " " << pCat->nFN << " " << pCat->nTN << " " << pCat->nSE << std::endl;
    oMetricsOutput << std::endl << std::endl;
    oMetricsOutput << std::fixed << std::setprecision(8);
    oMetricsOutput << "Sequence Metrics :" << std::endl;
    oMetricsOutput << "           Rcl        Spc        FPR        FNR        PBC        Prc        FM         MCC        " << std::endl;
    for(size_t i=0; i<pCat->m_vpSequences.size(); ++i) {
        MetricsCalculator tmp(pCat->m_vpSequences[i]);
        std::string sName = pCat->m_vpSequences[i]->m_sName;
        if(sName.size()>10)
            sName = sName.substr(0,10);
        else if(sName.size()<10)
            sName += std::string(10-sName.size(),' ');
        oMetricsOutput << sName << " " << tmp.m_oMetrics.dRecall << " " << tmp.m_oMetrics.dSpecficity << " " << tmp.m_oMetrics.dFPR << " " << tmp.m_oMetrics.dFNR << " " << tmp.m_oMetrics.dPBC << " " << tmp.m_oMetrics.dPrecision << " " << tmp.m_oMetrics.dFMeasure << " " << tmp.m_oMetrics.dMCC << std::endl;
    }
    oMetricsOutput << "--------------------------------------------------------------------------------------------------" << std::endl;
    MetricsCalculator all(pCat, USE_AVERAGE_METRICS);
    const std::string sCurrCatName = pCat->m_sName.size()>12?pCat->m_sName.substr(0,12):pCat->m_sName;
    std::cout << "\t" << std::setfill(' ') << std::setw(12) << sCurrCatName << " : Rcl=" << std::fixed << std::setprecision(4) << all.m_oMetrics.dRecall << " Prc=" << all.m_oMetrics.dPrecision << " FM=" << all.m_oMetrics.dFMeasure << " MCC=" << all.m_oMetrics.dMCC << std::endl;
    oMetricsOutput << std::string(USE_AVERAGE_METRICS?"averaged   ":"cumulative ") << all.m_oMetrics.dRecall << " " << all.m_oMetrics.dSpecficity << " " << all.m_oMetrics.dFPR << " " << all.m_oMetrics.dFNR << " " << all.m_oMetrics.dPBC << " " << all.m_oMetrics.dPrecision << " " << all.m_oMetrics.dFMeasure << " " << all.m_oMetrics.dMCC << std::endl;
    oMetricsOutput << std::endl << std::endl;
    oMetricsOutput << "All Sequences Average FPS: " << all.m_oMetrics.dFPS << std::endl;
    oMetricsOutput.close();
}

void DatasetUtils::WriteMetrics(const std::string sResultsFileName, std::vector<CategoryInfo*>& vpCat, double dTotalFPS) {
    std::ofstream oMetricsOutput(sResultsFileName);
    std::sort(vpCat.begin(),vpCat.end(),&CategoryInfo::compare);
    oMetricsOutput << std::fixed << std::setprecision(8);
    oMetricsOutput << "Overall results :" << std::endl;
    oMetricsOutput << std::endl;
    oMetricsOutput << std::string(USE_AVERAGE_METRICS?"Averaged":"Cumulative") << " metrics :" << std::endl;
    oMetricsOutput << "           Rcl        Spc        FPR        FNR        PBC        Prc        FM         MCC        " << std::endl;
    for(size_t i=0; i<vpCat.size(); ++i) {
        if(!vpCat[i]->m_vpSequences.empty()) {
            MetricsCalculator tmp(vpCat[i],USE_AVERAGE_METRICS);
            std::string sName = vpCat[i]->m_sName;
            if(sName.size()>10)
                sName = sName.substr(0,10);
            else if(sName.size()<10)
                sName += std::string(10-sName.size(),' ');
            oMetricsOutput << sName << " " << tmp.m_oMetrics.dRecall << " " << tmp.m_oMetrics.dSpecficity << " " << tmp.m_oMetrics.dFPR << " " << tmp.m_oMetrics.dFNR << " " << tmp.m_oMetrics.dPBC << " " << tmp.m_oMetrics.dPrecision << " " << tmp.m_oMetrics.dFMeasure << " " << tmp.m_oMetrics.dMCC << std::endl;
        }
    }
    oMetricsOutput << "--------------------------------------------------------------------------------------------------" << std::endl;
    MetricsCalculator all(vpCat,USE_AVERAGE_METRICS);
    oMetricsOutput << "overall    " << all.m_oMetrics.dRecall << " " << all.m_oMetrics.dSpecficity << " " << all.m_oMetrics.dFPR << " " << all.m_oMetrics.dFNR << " " << all.m_oMetrics.dPBC << " " << all.m_oMetrics.dPrecision << " " << all.m_oMetrics.dFMeasure << " " << all.m_oMetrics.dMCC << std::endl;
    oMetricsOutput << std::endl << std::endl;
    oMetricsOutput << "All Sequences Average FPS: " << all.m_oMetrics.dFPS << std::endl;
    oMetricsOutput << "Total FPS: " << dTotalFPS << std::endl;
    oMetricsOutput.close();
}

void DatasetUtils::CalcMetricsFromResult(const cv::Mat& oSegmResFrame, const cv::Mat& oGTFrame, const cv::Mat& oROI, uint64_t& nTP, uint64_t& nTN, uint64_t& nFP, uint64_t& nFN, uint64_t& nSE) {
    CV_DbgAssert(oSegmResFrame.type()==CV_8UC1 && oGTFrame.type()==CV_8UC1 && oROI.type()==CV_8UC1);
    CV_DbgAssert(oSegmResFrame.size()==oGTFrame.size() && oSegmResFrame.size()==oROI.size());
    const size_t step_row = oSegmResFrame.step.p[0];
    for(size_t i=0; i<(size_t)oSegmResFrame.rows; ++i) {
        const size_t idx_nstep = step_row*i;
        const uchar* input_step_ptr = oSegmResFrame.data+idx_nstep;
        const uchar* gt_step_ptr = oGTFrame.data+idx_nstep;
        const uchar* roi_step_ptr = oROI.data+idx_nstep;
        for(int j=0; j<oSegmResFrame.cols; ++j) {
            if( gt_step_ptr[j]!=g_nCDnetOutOfScope &&
                gt_step_ptr[j]!=g_nCDnetUnknown &&
                roi_step_ptr[j]!=g_nCDnetNegative ) {
                if(input_step_ptr[j]==g_nCDnetPositive) {
                    if(gt_step_ptr[j]==g_nCDnetPositive)
                        ++nTP;
                    else // gt_step_ptr[j]==g_nCDnetNegative
                        ++nFP;
                }
                else { // input_step_ptr[j]==g_nCDnetNegative
                    if(gt_step_ptr[j]==g_nCDnetPositive)
                        ++nFN;
                    else // gt_step_ptr[j]==g_nCDnetNegative
                        ++nTN;
                }
                if(gt_step_ptr[j]==g_nCDnetShadow) {
                    if(input_step_ptr[j]==g_nCDnetPositive)
                        ++nSE;
                }
            }
        }
    }
}

inline DatasetUtils::CommonMetrics CalcMetricsFromCounts(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN, uint64_t /*nSE*/, double dFPS) {
    DatasetUtils::CommonMetrics res;
    res.dRecall = DatasetUtils::CalcMetric_Recall(nTP,nTN,nFP,nFN);
    res.dSpecficity = DatasetUtils::CalcMetric_Specificity(nTP,nTN,nFP,nFN);
    res.dFPR = DatasetUtils::CalcMetric_FalsePositiveRate(nTP,nTN,nFP,nFN);
    res.dFNR = DatasetUtils::CalcMetric_FalseNegativeRate(nTP,nTN,nFP,nFN);
    res.dPBC = DatasetUtils::CalcMetric_PercentBadClassifs(nTP,nTN,nFP,nFN);
    res.dPrecision = DatasetUtils::CalcMetric_Precision(nTP,nTN,nFP,nFN);
    res.dFMeasure = DatasetUtils::CalcMetric_FMeasure(nTP,nTN,nFP,nFN);
    res.dMCC = DatasetUtils::CalcMetric_MatthewsCorrCoeff(nTP,nTN,nFP,nFN);
    res.dFPS = dFPS;
    return res;
}

inline DatasetUtils::CommonMetrics CalcMetricsFromCategory(const DatasetUtils::CategoryInfo* pCat, bool bAverage) {
    DatasetUtils::CommonMetrics res;
    if(!bAverage) {
        res.dRecall = DatasetUtils::CalcMetric_Recall(pCat->nTP,pCat->nTN,pCat->nFP,pCat->nFN);
        res.dSpecficity = DatasetUtils::CalcMetric_Specificity(pCat->nTP,pCat->nTN,pCat->nFP,pCat->nFN);
        res.dFPR = DatasetUtils::CalcMetric_FalsePositiveRate(pCat->nTP,pCat->nTN,pCat->nFP,pCat->nFN);
        res.dFNR = DatasetUtils::CalcMetric_FalseNegativeRate(pCat->nTP,pCat->nTN,pCat->nFP,pCat->nFN);
        res.dPBC = DatasetUtils::CalcMetric_PercentBadClassifs(pCat->nTP,pCat->nTN,pCat->nFP,pCat->nFN);
        res.dPrecision = DatasetUtils::CalcMetric_Precision(pCat->nTP,pCat->nTN,pCat->nFP,pCat->nFN);
        res.dFMeasure = DatasetUtils::CalcMetric_FMeasure(pCat->nTP,pCat->nTN,pCat->nFP,pCat->nFN);
        res.dMCC = DatasetUtils::CalcMetric_MatthewsCorrCoeff(pCat->nTP,pCat->nTN,pCat->nFP,pCat->nFN);
        res.dFPS = pCat->m_dAvgFPS;
    }
    else {
        res.dRecall = 0;
        res.dSpecficity = 0;
        res.dFPR = 0;
        res.dFNR = 0;
        res.dPBC = 0;
        res.dPrecision = 0;
        res.dFMeasure = 0;
        res.dMCC = 0;
        res.dFPS = 0;
        const size_t nSeq = pCat->m_vpSequences.size();
        for(size_t i=0; i<nSeq; ++i) {
            const DatasetUtils::SequenceInfo* pCurrSeq = pCat->m_vpSequences[i];
            res.dRecall += DatasetUtils::CalcMetric_Recall(pCurrSeq->nTP,pCurrSeq->nTN,pCurrSeq->nFP,pCurrSeq->nFN);
            res.dSpecficity += DatasetUtils::CalcMetric_Specificity(pCurrSeq->nTP,pCurrSeq->nTN,pCurrSeq->nFP,pCurrSeq->nFN);
            res.dFPR += DatasetUtils::CalcMetric_FalsePositiveRate(pCurrSeq->nTP,pCurrSeq->nTN,pCurrSeq->nFP,pCurrSeq->nFN);
            res.dFNR += DatasetUtils::CalcMetric_FalseNegativeRate(pCurrSeq->nTP,pCurrSeq->nTN,pCurrSeq->nFP,pCurrSeq->nFN);
            res.dPBC += DatasetUtils::CalcMetric_PercentBadClassifs(pCurrSeq->nTP,pCurrSeq->nTN,pCurrSeq->nFP,pCurrSeq->nFN);
            res.dPrecision += DatasetUtils::CalcMetric_Precision(pCurrSeq->nTP,pCurrSeq->nTN,pCurrSeq->nFP,pCurrSeq->nFN);
            res.dFMeasure += DatasetUtils::CalcMetric_FMeasure(pCurrSeq->nTP,pCurrSeq->nTN,pCurrSeq->nFP,pCurrSeq->nFN);
            res.dMCC += DatasetUtils::CalcMetric_MatthewsCorrCoeff(pCurrSeq->nTP,pCurrSeq->nTN,pCurrSeq->nFP,pCurrSeq->nFN);
            res.dFPS += pCurrSeq->m_dAvgFPS;
        }
        res.dRecall /= nSeq;
        res.dSpecficity /= nSeq;
        res.dFPR /= nSeq;
        res.dFNR /= nSeq;
        res.dPBC /= nSeq;
        res.dPrecision /= nSeq;
        res.dFMeasure /= nSeq;
        res.dMCC /= nSeq;
        res.dFPS /= nSeq;
    }
    return res;
}

inline DatasetUtils::CommonMetrics CalcMetricsFromCategories(const std::vector<DatasetUtils::CategoryInfo*>& vpCat, bool bAverage) {
    DatasetUtils::CommonMetrics res;
    const size_t nCat = vpCat.size();
    size_t nBadCat = 0;
    if(!bAverage) {
        uint64_t nGlobalTP=0, nGlobalTN=0, nGlobalFP=0, nGlobalFN=0, nGlobalSE=0;
        res.dFPS=0;
        for(size_t i=0; i<nCat; ++i) {
            if(vpCat[i]->m_vpSequences.empty()) {
                ++nBadCat;
            }
            else {
                nGlobalTP += vpCat[i]->nTP;
                nGlobalTN += vpCat[i]->nTN;
                nGlobalFP += vpCat[i]->nFP;
                nGlobalFN += vpCat[i]->nFN;
                nGlobalSE += vpCat[i]->nSE;
                res.dFPS += vpCat[i]->m_dAvgFPS;
            }
        }
        CV_Assert(nBadCat<nCat);
        res.dRecall = DatasetUtils::CalcMetric_Recall(nGlobalTP,nGlobalTN,nGlobalFP,nGlobalFN);
        res.dSpecficity = DatasetUtils::CalcMetric_Specificity(nGlobalTP,nGlobalTN,nGlobalFP,nGlobalFN);
        res.dFPR = DatasetUtils::CalcMetric_FalsePositiveRate(nGlobalTP,nGlobalTN,nGlobalFP,nGlobalFN);
        res.dFNR = DatasetUtils::CalcMetric_FalseNegativeRate(nGlobalTP,nGlobalTN,nGlobalFP,nGlobalFN);
        res.dPBC = DatasetUtils::CalcMetric_PercentBadClassifs(nGlobalTP,nGlobalTN,nGlobalFP,nGlobalFN);
        res.dPrecision = DatasetUtils::CalcMetric_Precision(nGlobalTP,nGlobalTN,nGlobalFP,nGlobalFN);
        res.dFMeasure = DatasetUtils::CalcMetric_FMeasure(nGlobalTP,nGlobalTN,nGlobalFP,nGlobalFN);
        res.dMCC = DatasetUtils::CalcMetric_MatthewsCorrCoeff(nGlobalTP,nGlobalTN,nGlobalFP,nGlobalFN);
        res.dFPS /= (nCat-nBadCat);
    }
    else {
        res.dRecall = 0;
        res.dSpecficity = 0;
        res.dFPR = 0;
        res.dFNR = 0;
        res.dPBC = 0;
        res.dPrecision = 0;
        res.dFMeasure = 0;
        res.dMCC = 0;
        res.dFPS = 0;
        for(size_t i=0; i<nCat; ++i) {
            if(vpCat[i]->m_vpSequences.empty())
                ++nBadCat;
            else {
                DatasetUtils::CommonMetrics curr = CalcMetricsFromCategory(vpCat[i],true);
                res.dRecall += curr.dRecall;
                res.dSpecficity += curr.dSpecficity;
                res.dFPR += curr.dFPR;
                res.dFNR += curr.dFNR;
                res.dPBC += curr.dPBC;
                res.dPrecision += curr.dPrecision;
                res.dFMeasure += curr.dFMeasure;
                res.dMCC += curr.dMCC;
                res.dFPS += curr.dFPS;
            }
        }
        CV_Assert(nBadCat<nCat);
        res.dRecall /= (nCat-nBadCat);
        res.dSpecficity /= (nCat-nBadCat);
        res.dFPR /= (nCat-nBadCat);
        res.dFNR /= (nCat-nBadCat);
        res.dPBC /= (nCat-nBadCat);
        res.dPrecision /= (nCat-nBadCat);
        res.dFMeasure /= (nCat-nBadCat);
        res.dMCC /= (nCat-nBadCat);
        res.dFPS /= (nCat-nBadCat);
    }
    return res;
}

cv::Mat DatasetUtils::GetColoredSegmFrameFromResult(const cv::Mat& oSegmResFrame, const cv::Mat& oGTFrame, const cv::Mat& oROI) {
    CV_DbgAssert(oSegmResFrame.type()==CV_8UC1 && oGTFrame.type()==CV_8UC1 && oROI.type()==CV_8UC1);
    CV_DbgAssert(oSegmResFrame.size()==oGTFrame.size() && oSegmResFrame.size()==oROI.size());
    cv::Mat oResult(oSegmResFrame.size(),CV_8UC3,cv::Scalar_<uchar>(0));
    const size_t step_row = oSegmResFrame.step.p[0];
    for(size_t i=0; i<(size_t)oSegmResFrame.rows; ++i) {
        const size_t idx_nstep = step_row*i;
        const uchar* input_step_ptr = oSegmResFrame.data+idx_nstep;
        const uchar* gt_step_ptr = oGTFrame.data+idx_nstep;
        const uchar* roi_step_ptr = oROI.data+idx_nstep;
        uchar* res_step_ptr = oResult.data+idx_nstep*3;
        for(int j=0; j<oSegmResFrame.cols; ++j) {
            if( gt_step_ptr[j]!=g_nCDnetOutOfScope &&
                gt_step_ptr[j]!=g_nCDnetUnknown &&
                roi_step_ptr[j]!=g_nCDnetNegative ) {
                if(input_step_ptr[j]==g_nCDnetPositive) {
                    if(gt_step_ptr[j]==g_nCDnetPositive)
                        res_step_ptr[j*3+1] = UCHAR_MAX;
                    else if(gt_step_ptr[j]==g_nCDnetNegative)
                        res_step_ptr[j*3+2] = UCHAR_MAX;
                    else if(gt_step_ptr[j]==g_nCDnetShadow) {
                        res_step_ptr[j*3+1] = UCHAR_MAX/2;
                        res_step_ptr[j*3+2] = UCHAR_MAX;
                    }
                    else {
                        for(size_t c=0; c<3; ++c)
                            res_step_ptr[j*3+c] = UCHAR_MAX/3;
                    }
                }
                else { // input_step_ptr[j]==g_nCDnetNegative
                    if(gt_step_ptr[j]==g_nCDnetPositive) {
                        res_step_ptr[j*3] = UCHAR_MAX/2;
                        res_step_ptr[j*3+2] = UCHAR_MAX;
                    }
                }
            }
            else if(roi_step_ptr[j]==g_nCDnetNegative) {
                for(size_t c=0; c<3; ++c)
                    res_step_ptr[j*3+c] = UCHAR_MAX/2;
            }
            else {
                for(size_t c=0; c<3; ++c)
                    res_step_ptr[j*3+c] = input_step_ptr[j];
            }
        }
    }
    return oResult;
}

DatasetUtils::MetricsCalculator::MetricsCalculator(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN, uint64_t nSE)
    :    m_oMetrics(CalcMetricsFromCounts(nTP,nTN,nFP,nFN,nSE,0)),m_bAveraged(false) {}

DatasetUtils::MetricsCalculator::MetricsCalculator(const SequenceInfo* pSeq)
    :    m_oMetrics(CalcMetricsFromCounts(pSeq->nTP,pSeq->nTN,pSeq->nFP,pSeq->nFN,pSeq->nSE,pSeq->m_dAvgFPS)),m_bAveraged(false) {}

DatasetUtils::MetricsCalculator::MetricsCalculator(const CategoryInfo* pCat, bool bAverage)
    :    m_oMetrics(CalcMetricsFromCategory(pCat,bAverage)),m_bAveraged(bAverage) {CV_Assert(!pCat->m_vpSequences.empty());}

DatasetUtils::MetricsCalculator::MetricsCalculator(const std::vector<CategoryInfo*>& vpCat, bool bAverage)
    :    m_oMetrics(CalcMetricsFromCategories(vpCat,bAverage)),m_bAveraged(bAverage) {CV_Assert(!vpCat.empty());}

DatasetUtils::CategoryInfo::CategoryInfo(const std::string& sName, const std::string& sDirectoryPath,
                                         DatasetUtils::eAvailableDatasetsID eDatasetID,
                                         const char* asGrayscaleDirNameTokens[])
    :    m_sName(sName)
        ,m_eDatasetID(eDatasetID)
        ,nTP(0),nTN(0),nFP(0),nFN(0),nSE(0)
        ,m_dAvgFPS(-1) {
    std::cout << "\tParsing dir '" << sDirectoryPath << "' for category '" << m_sName << "'... ";
    std::vector<std::string> vsSequencePaths;
    if(m_eDatasetID==eDataset_CDnet || m_eDatasetID==eDataset_Wallflower || m_eDatasetID==eDataset_PETS2001_D3TC1) {
        // all subdirs are considered sequence directories
        PlatformUtils::GetSubDirsFromDir(sDirectoryPath,vsSequencePaths);
        std::cout << "(" << vsSequencePaths.size() << " sequences)" << std::endl;
    }
    else if(m_eDatasetID==eDataset_LITIV_Registr01) {
        // all subdirs should contain individual video tracks in separate modalities
        PlatformUtils::GetSubDirsFromDir(sDirectoryPath,vsSequencePaths);
        std::cout << "(" << vsSequencePaths.size() << " tracks)" << std::endl;
    }
    else if(m_eDatasetID==eDataset_GenericSegmentationTest) {
        // all files are considered sequences
        PlatformUtils::GetFilesFromDir(sDirectoryPath,vsSequencePaths);
        std::cout << "(" << vsSequencePaths.size() << " sequences)" << std::endl;
    }
    else
        throw std::runtime_error(std::string("Unknown dataset type, cannot use any known parsing strategy."));
    for(auto iter=vsSequencePaths.begin(); iter!=vsSequencePaths.end(); ++iter) {
        bool bForceGrayscale = false;
        if(asGrayscaleDirNameTokens) {
            for(size_t i=0; i<sizeof(asGrayscaleDirNameTokens)/sizeof(char*) && !bForceGrayscale; ++i)
                bForceGrayscale = iter->find(asGrayscaleDirNameTokens[i])!=std::string::npos;
        }
        const size_t pos = iter->find_last_of("/\\");
        if(pos==std::string::npos)
            m_vpSequences.push_back(new SequenceInfo(*iter,*iter,this,bForceGrayscale));
        else
            m_vpSequences.push_back(new SequenceInfo(iter->substr(pos+1),*iter,this,bForceGrayscale));
    }
}

DatasetUtils::CategoryInfo::~CategoryInfo() {
    for(size_t i=0; i<m_vpSequences.size(); i++)
        delete m_vpSequences[i];
}

DatasetUtils::SequenceInfo::SequenceInfo(const std::string& sName, const std::string& sPath, CategoryInfo* pParent, bool bForceGrayscale)
    :    m_sName(sName)
        ,m_sPath(sPath)
        ,m_eDatasetID(pParent?pParent->m_eDatasetID:eDataset_GenericSegmentationTest)
        ,nTP(0),nTN(0),nFP(0),nFN(0),nSE(0)
        ,m_dAvgFPS(-1)
        ,m_dExpectedLoad(0)
        ,m_dExpectedROILoad(0)
        ,m_pParent(pParent)
#if USE_PRECACHED_IO
        ,m_bIsPrecaching(false)
        ,m_nRequestInputFrameIndex(SIZE_MAX)
        ,m_nRequestGTFrameIndex(SIZE_MAX)
        ,m_nNextExpectedInputFrameIdx(0)
        ,m_nNextExpectedGTFrameIdx(0)
        ,m_nNextPrecachedInputFrameIdx(0)
        ,m_nNextPrecachedGTFrameIdx(0)
#else //!USE_PRECACHED_IO
        ,m_nLastReqInputFrameIndex(UINT_MAX)
        ,m_nLastReqGTFrameIndex(UINT_MAX)
#endif //!USE_PRECACHED_IO
        ,m_nNextExpectedVideoReaderFrameIdx(0)
        ,m_nTotalNbFrames(0)
        ,m_nIMReadInputFlags(bForceGrayscale?cv::IMREAD_GRAYSCALE:cv::IMREAD_COLOR) {
    if(m_eDatasetID==eDataset_CDnet) {
        std::vector<std::string> vsSubDirs;
        PlatformUtils::GetSubDirsFromDir(m_sPath,vsSubDirs);
        auto gtDir = std::find(vsSubDirs.begin(),vsSubDirs.end(),m_sPath+"/groundtruth");
        auto inputDir = std::find(vsSubDirs.begin(),vsSubDirs.end(),m_sPath+"/input");
        if(gtDir==vsSubDirs.end() || inputDir==vsSubDirs.end())
            throw std::runtime_error(std::string("Sequence at ") + m_sPath + " did not possess the required groundtruth and input directories.");
        PlatformUtils::GetFilesFromDir(*inputDir,m_vsInputFramePaths);
        PlatformUtils::GetFilesFromDir(*gtDir,m_vsGTFramePaths);
        if(m_vsGTFramePaths.size()!=m_vsInputFramePaths.size())
            throw std::runtime_error(std::string("Sequence at ") + m_sPath + " did not possess same amount of GT & input frames.");
        m_oROI = cv::imread(m_sPath+"/ROI.bmp",cv::IMREAD_GRAYSCALE);
        if(m_oROI.empty())
            throw std::runtime_error(std::string("Sequence at ") + m_sPath + " did not possess a ROI.bmp file.");
        m_oROI = m_oROI>0;
        m_oSize = m_oROI.size();
        m_nTotalNbFrames = m_vsInputFramePaths.size();
        m_dExpectedLoad = (double)m_oSize.height*m_oSize.width*m_nTotalNbFrames*(m_nIMReadInputFlags==cv::IMREAD_COLOR?2:1);
        m_dExpectedROILoad = (double)cv::countNonZero(m_oROI)*m_nTotalNbFrames*(m_nIMReadInputFlags==cv::IMREAD_COLOR?2:1);
        // note: in this case, no need to use m_vnTestGTIndexes since all # of gt frames == # of test frames (but we assume the frames returned by 'GetFilesFromDir' are ordered correctly...)
    }
    else if(m_eDatasetID==eDataset_Wallflower) {
        std::vector<std::string> vsImgPaths;
        PlatformUtils::GetFilesFromDir(m_sPath,vsImgPaths);
        bool bFoundScript=false, bFoundGTFile=false;
        const std::string sGTFilePrefix("hand_segmented_");
        const size_t nInputFileNbDecimals = 5;
        const std::string sInputFileSuffix(".bmp");
        for(auto iter=vsImgPaths.begin(); iter!=vsImgPaths.end(); ++iter) {
            if(*iter==m_sPath+"/script.txt")
                bFoundScript = true;
            else if(iter->find(sGTFilePrefix)!=std::string::npos) {
                m_mTestGTIndexes.insert(std::pair<size_t,size_t>(atoi(iter->substr(iter->find(sGTFilePrefix)+sGTFilePrefix.size(),nInputFileNbDecimals).c_str()),m_vsGTFramePaths.size()));
                m_vsGTFramePaths.push_back(*iter);
                bFoundGTFile = true;
            }
            else {
                if(iter->find(sInputFileSuffix)!=iter->size()-sInputFileSuffix.size())
                    throw std::runtime_error(std::string("Sequence directory at ") + m_sPath + " contained an unknown file ('" + *iter + "')");
                m_vsInputFramePaths.push_back(*iter);
            }
        }
        if(!bFoundGTFile || !bFoundScript || m_vsInputFramePaths.empty() || m_vsGTFramePaths.size()!=1)
            throw std::runtime_error(std::string("Sequence at ") + m_sPath + " did not possess the required groundtruth and input files.");
        cv::Mat oTempImg = cv::imread(m_vsGTFramePaths[0]);
        if(oTempImg.empty())
            throw std::runtime_error(std::string("Sequence at ") + m_sPath + " did not possess a valid GT file.");
        m_oROI = cv::Mat(oTempImg.size(),CV_8UC1,cv::Scalar(g_nCDnetPositive));
        m_oSize = oTempImg.size();
        m_nTotalNbFrames = m_vsInputFramePaths.size();
        m_dExpectedLoad = m_dExpectedROILoad = (double)m_oSize.height*m_oSize.width*m_nTotalNbFrames*(m_nIMReadInputFlags==cv::IMREAD_COLOR?2:1);
    }
    else if(m_eDatasetID==eDataset_PETS2001_D3TC1) {
        std::vector<std::string> vsVideoSeqPaths;
        PlatformUtils::GetFilesFromDir(m_sPath,vsVideoSeqPaths);
        if(vsVideoSeqPaths.size()!=1)
            throw std::runtime_error(std::string("Bad subdirectory ('")+m_sPath+std::string("') for PETS2001 parsing (should contain only one video sequence file)"));
        std::vector<std::string> vsGTSubdirPaths;
        PlatformUtils::GetSubDirsFromDir(m_sPath,vsGTSubdirPaths);
        if(vsGTSubdirPaths.size()!=1)
            throw std::runtime_error(std::string("Bad subdirectory ('")+m_sPath+std::string("') for PETS2001 parsing (should contain only one GT subdir)"));
        m_voVideoReader.open(vsVideoSeqPaths[0]);
        if(!m_voVideoReader.isOpened())
            throw std::runtime_error(std::string("Bad video file ('")+vsVideoSeqPaths[0]+std::string("'), could not be opened."));
        PlatformUtils::GetFilesFromDir(vsGTSubdirPaths[0],m_vsGTFramePaths);
        if(m_vsGTFramePaths.empty())
            throw std::runtime_error(std::string("Sequence at ") + m_sPath + " did not possess any valid GT frames.");
        const std::string sGTFilePrefix("image_");
        const size_t nInputFileNbDecimals = 4;
        for(auto iter=m_vsGTFramePaths.begin(); iter!=m_vsGTFramePaths.end(); ++iter)
            m_mTestGTIndexes.insert(std::pair<size_t,size_t>(atoi(iter->substr(iter->find(sGTFilePrefix)+sGTFilePrefix.size(),nInputFileNbDecimals).c_str()),iter-m_vsGTFramePaths.begin()));
        cv::Mat oTempImg = cv::imread(m_vsGTFramePaths[0]);
        if(oTempImg.empty())
            throw std::runtime_error(std::string("Sequence at ") + m_sPath + " did not possess valid GT file(s).");
        m_oROI = cv::Mat(oTempImg.size(),CV_8UC1,cv::Scalar(g_nCDnetPositive));
        m_oSize = oTempImg.size();
        m_nNextExpectedVideoReaderFrameIdx = 0;
        m_nTotalNbFrames = (size_t)m_voVideoReader.get(CV_CAP_PROP_FRAME_COUNT);
        m_dExpectedLoad = m_dExpectedROILoad = (double)m_oSize.height*m_oSize.width*m_nTotalNbFrames*(m_nIMReadInputFlags==cv::IMREAD_COLOR?2:1);
    }
    else if(m_eDatasetID==eDataset_LITIV_Registr01) {
        m_voVideoReader.open(m_sPath+"/"+m_pParent->m_sName+"_"+m_sName+".avi");
        if(!m_voVideoReader.isOpened())
            throw std::runtime_error(std::string("Bad video file ('")+m_sPath+std::string("'), could not be opened."));
        m_voVideoReader.set(CV_CAP_PROP_POS_FRAMES,0);
        cv::Mat oTempImg;
        m_voVideoReader >> oTempImg;
        m_voVideoReader.set(CV_CAP_PROP_POS_FRAMES,0);
        if(oTempImg.empty())
            throw std::runtime_error(std::string("Bad video file ('")+m_sPath+std::string("'), could not be read."));
        m_oROI = cv::Mat(oTempImg.size(),CV_8UC1,cv::Scalar(g_nCDnetPositive));
        m_oSize = oTempImg.size();
        m_nNextExpectedVideoReaderFrameIdx = -1;
        m_nTotalNbFrames = (size_t)m_voVideoReader.get(CV_CAP_PROP_FRAME_COUNT);
        m_dExpectedLoad = m_dExpectedROILoad = (double)m_oSize.height*m_oSize.width*m_nTotalNbFrames*(m_nIMReadInputFlags==cv::IMREAD_COLOR?2:1);
    }
    else if(m_eDatasetID==eDataset_GenericSegmentationTest) {
        m_voVideoReader.open(m_sPath);
        if(!m_voVideoReader.isOpened())
            throw std::runtime_error(std::string("Bad video file ('")+m_sPath+std::string("'), could not be opened."));
        m_voVideoReader.set(CV_CAP_PROP_POS_FRAMES,0);
        cv::Mat oTempImg;
        m_voVideoReader >> oTempImg;
        m_voVideoReader.set(CV_CAP_PROP_POS_FRAMES,0);
        if(oTempImg.empty())
            throw std::runtime_error(std::string("Bad video file ('")+m_sPath+std::string("'), could not be read."));
        m_oROI = cv::Mat(oTempImg.size(),CV_8UC1,cv::Scalar(g_nCDnetPositive));
        m_oSize = oTempImg.size();
        m_nNextExpectedVideoReaderFrameIdx = 0;
        m_nTotalNbFrames = (size_t)m_voVideoReader.get(CV_CAP_PROP_FRAME_COUNT);
        m_dExpectedLoad = m_dExpectedROILoad = (double)m_oSize.height*m_oSize.width*m_nTotalNbFrames*(m_nIMReadInputFlags==cv::IMREAD_COLOR?2:1);
    }
    else
        throw std::runtime_error(std::string("Unknown dataset type, cannot use any known parsing strategy."));
#if USE_PRECACHED_IO
    CV_Assert(MAX_NB_PRECACHED_FRAMES>1);
    CV_Assert(PRECACHE_REFILL_THRESHOLD>1 && PRECACHE_REFILL_THRESHOLD<MAX_NB_PRECACHED_FRAMES);
    CV_Assert(REQUEST_TIMEOUT_MS>0);
    CV_Assert(QUERY_TIMEOUT_MS>0);
#endif //USE_PRECACHED_IO
}

DatasetUtils::SequenceInfo::~SequenceInfo() {
#if USE_PRECACHED_IO
    StopPrecaching();
#endif //USE_PRECACHED_IO
}

size_t DatasetUtils::SequenceInfo::GetNbInputFrames() const {
    return m_nTotalNbFrames;
}

size_t DatasetUtils::SequenceInfo::GetNbGTFrames() const {
    return m_mTestGTIndexes.empty()?m_vsGTFramePaths.size():m_mTestGTIndexes.size();
}

cv::Size DatasetUtils::SequenceInfo::GetFrameSize() const {
    return m_oSize;
}

const cv::Mat& DatasetUtils::SequenceInfo::GetSequenceROI() const {
    return m_oROI;
}

std::vector<cv::KeyPoint> DatasetUtils::SequenceInfo::GetKeyPointsFromROI() const {
    std::vector<cv::KeyPoint> voNewKPs;
    cv::DenseFeatureDetector oKPDDetector(1.f, 1, 1.f, 1, 0, true, false);
    voNewKPs.reserve(m_oROI.rows*m_oROI.cols);
    oKPDDetector.detect(cv::Mat(m_oROI.size(),m_oROI.type()),voNewKPs);
    ValidateKeyPoints(voNewKPs);
    return voNewKPs;
}

void DatasetUtils::SequenceInfo::ValidateKeyPoints(std::vector<cv::KeyPoint>& voKPs) const {
    std::vector<cv::KeyPoint> voNewKPs;
    for(size_t k=0; k<voKPs.size(); ++k) {
        if(m_oROI.at<uchar>(voKPs[k].pt)>0)
            voNewKPs.push_back(voKPs[k]);
    }
    voKPs = voNewKPs;
}

cv::Mat DatasetUtils::SequenceInfo::GetInputFrameFromIndex_Internal(size_t nFrameIdx) {
    CV_DbgAssert(nFrameIdx<m_nTotalNbFrames);
    cv::Mat oFrame;
    if(m_eDatasetID==eDataset_CDnet || m_eDatasetID==eDataset_Wallflower)
        oFrame = cv::imread(m_vsInputFramePaths[nFrameIdx],m_nIMReadInputFlags);
    else if(m_eDatasetID==eDataset_PETS2001_D3TC1 || m_eDatasetID==eDataset_LITIV_Registr01 || m_eDatasetID==eDataset_GenericSegmentationTest) {
        if(m_nNextExpectedVideoReaderFrameIdx!=nFrameIdx) {
            std::cout << "test" << std::endl;
            m_voVideoReader >> oFrame;
            cv::imshow("test1",oFrame);
            m_voVideoReader.set(CV_CAP_PROP_POS_FRAMES,(double)nFrameIdx);
            m_nNextExpectedVideoReaderFrameIdx = nFrameIdx+1;
        }
        else
            ++m_nNextExpectedVideoReaderFrameIdx;
        m_voVideoReader >> oFrame;
        cv::imshow("test2",oFrame);//@@@@
        cv::waitKey(0);
    }
    CV_DbgAssert(oFrame.size()==m_oSize);
    return oFrame;
}

cv::Mat DatasetUtils::SequenceInfo::GetGTFrameFromIndex_Internal(size_t nFrameIdx) {
    CV_DbgAssert(nFrameIdx<m_nTotalNbFrames);
    cv::Mat oFrame;
    if(m_eDatasetID==eDataset_CDnet)
        oFrame = cv::imread(m_vsGTFramePaths[nFrameIdx],cv::IMREAD_GRAYSCALE);
    else if(m_eDatasetID==eDataset_Wallflower || m_eDatasetID==eDataset_PETS2001_D3TC1) {
        auto res = m_mTestGTIndexes.find(nFrameIdx);
        if(res!=m_mTestGTIndexes.end())
            oFrame = cv::imread(m_vsGTFramePaths[res->second],cv::IMREAD_GRAYSCALE);
        else
            oFrame = cv::Mat(m_oSize,CV_8UC1,cv::Scalar(g_nCDnetOutOfScope));
    }
    else if(m_eDatasetID==eDataset_LITIV_Registr01 || m_eDatasetID==eDataset_GenericSegmentationTest) {
        oFrame = cv::Mat(m_oSize,CV_8UC1,cv::Scalar(g_nCDnetOutOfScope));
    }
    CV_DbgAssert(oFrame.size()==m_oSize);
    return oFrame;
}

const cv::Mat& DatasetUtils::SequenceInfo::GetInputFrameFromIndex(size_t nFrameIdx) {
#if USE_PRECACHED_IO
    if(!m_bIsPrecaching)
        throw std::runtime_error(m_sName + " [SequenceInfo] : Error, queried a frame before precaching was activated.");
#if PLATFORM_SUPPORTS_CPP11
    std::unique_lock<std::mutex> sync_lock(m_oInputFrameSyncMutex);
    m_nRequestInputFrameIndex = nFrameIdx;
    std::cv_status res;
    do {
        m_oInputFrameReqCondVar.notify_one();
        res = m_oInputFrameSyncCondVar.wait_for(sync_lock,std::chrono::milliseconds(REQUEST_TIMEOUT_MS));
        //if(res==std::cv_status::timeout) std::cout << " # retrying request..." << std::endl;
    } while(res==std::cv_status::timeout);
    return m_oReqInputFrame;
#elif PLATFORM_USES_WIN32API //&& !PLATFORM_SUPPORTS_CPP11
    EnterCriticalSection(&m_oInputFrameSyncMutex);
    m_nRequestInputFrameIndex = nFrameIdx;
    BOOL res;
    do {
        WakeConditionVariable(&m_oInputFrameReqCondVar);
        res = SleepConditionVariableCS(&m_oInputFrameSyncCondVar,&m_oInputFrameSyncMutex,REQUEST_TIMEOUT_MS);
        //if(!res) std::cout << " # retrying request..." << std::endl;
    } while(!res);
    LeaveCriticalSection(&m_oInputFrameSyncMutex);
    return m_oReqInputFrame;
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for precached io support on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#else //!USE_PRECACHED_IO
    if(m_nLastReqInputFrameIndex!=nFrameIdx) {
        m_oLastReqInputFrame = GetInputFrameFromIndex_Internal(nFrameIdx);
        m_nLastReqInputFrameIndex = nFrameIdx;
    }
    return m_oLastReqInputFrame;
#endif //!USE_PRECACHED_IO
}

const cv::Mat& DatasetUtils::SequenceInfo::GetGTFrameFromIndex(size_t nFrameIdx) {
#if USE_PRECACHED_IO
    if(!m_bIsPrecaching)
        throw std::runtime_error(m_sName + " [SequenceInfo] : Error, queried a frame before precaching was activated.");
#if PLATFORM_SUPPORTS_CPP11
    std::unique_lock<std::mutex> sync_lock(m_oGTFrameSyncMutex);
    m_nRequestGTFrameIndex = nFrameIdx;
    std::cv_status res;
    do {
        m_oGTFrameReqCondVar.notify_one();
        res = m_oGTFrameSyncCondVar.wait_for(sync_lock,std::chrono::milliseconds(REQUEST_TIMEOUT_MS));
        //if(res==std::cv_status::timeout) std::cout << " # retrying request..." << std::endl;
    } while(res==std::cv_status::timeout);
    return m_oReqGTFrame;
#elif PLATFORM_USES_WIN32API //&& !PLATFORM_SUPPORTS_CPP11
    EnterCriticalSection(&m_oGTFrameSyncMutex);
    m_nRequestGTFrameIndex = nFrameIdx;
    BOOL res;
    do {
        WakeConditionVariable(&m_oGTFrameReqCondVar);
        res = SleepConditionVariableCS(&m_oGTFrameSyncCondVar,&m_oGTFrameSyncMutex,REQUEST_TIMEOUT_MS);
        //if(!res) std::cout << " # retrying request..." << std::endl;
    } while(!res);
    LeaveCriticalSection(&m_oGTFrameSyncMutex);
    return m_oReqGTFrame;
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for precached io support on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#else //!USE_PRECACHED_IO
    if(m_nLastReqGTFrameIndex!=nFrameIdx) {
        m_oLastReqGTFrame = GetGTFrameFromIndex_Internal(nFrameIdx);
        m_nLastReqGTFrameIndex = nFrameIdx;
    }
    return m_oLastReqGTFrame;
#endif //!USE_PRECACHED_IO
}

#if USE_PRECACHED_IO

void DatasetUtils::SequenceInfo::PrecacheInputFrames() {
    srand((unsigned int)(time(nullptr)*m_nTotalNbFrames*m_sName.size()));
#if PLATFORM_SUPPORTS_CPP11
    std::unique_lock<std::mutex> sync_lock(m_oInputFrameSyncMutex);
#elif PLATFORM_USES_WIN32API //!PLATFORM_SUPPORTS_CPP11
    EnterCriticalSection(&m_oInputFrameSyncMutex);
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for precached io support on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
    const size_t nInitFramesToPrecache = MAX_NB_PRECACHED_FRAMES/2 + rand()%(MAX_NB_PRECACHED_FRAMES/2);
    //std::cout << " @ initializing precaching with " << nInitFramesToPrecache << " frames " << std::endl;
    while(m_qoInputFrameCache.size()<nInitFramesToPrecache && m_nNextPrecachedInputFrameIdx<m_nTotalNbFrames)
        m_qoInputFrameCache.push_back(GetInputFrameFromIndex_Internal(m_nNextPrecachedInputFrameIdx++));
    while(m_bIsPrecaching) {
#if PLATFORM_SUPPORTS_CPP11
        if(m_oInputFrameReqCondVar.wait_for(sync_lock,std::chrono::milliseconds(QUERY_TIMEOUT_MS))!=std::cv_status::timeout) {
#elif PLATFORM_USES_WIN32API //!PLATFORM_SUPPORTS_CPP11
        if(SleepConditionVariableCS(&m_oInputFrameReqCondVar,&m_oInputFrameSyncMutex,QUERY_TIMEOUT_MS)) {
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for precached io support on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
            CV_DbgAssert(m_nRequestInputFrameIndex<m_nTotalNbFrames);
            if(m_nRequestInputFrameIndex!=m_nNextExpectedInputFrameIdx-1) {
                if(!m_qoInputFrameCache.empty() && m_nRequestInputFrameIndex==m_nNextExpectedInputFrameIdx) {
                    m_oReqInputFrame = m_qoInputFrameCache.front();
                    m_qoInputFrameCache.pop_front();
                }
                else {
                    if(!m_qoInputFrameCache.empty()) {
                        //std::cout << " @ answering request manually, out of order (req=" << m_nRequestInputFrameIndex << ", expected=" << m_nNextExpectedInputFrameIdx <<") ";
                        CV_DbgAssert((m_nNextPrecachedInputFrameIdx-m_qoInputFrameCache.size())==m_nNextExpectedInputFrameIdx);
                        if(m_nRequestInputFrameIndex<m_nNextPrecachedInputFrameIdx && m_nRequestInputFrameIndex>m_nNextExpectedInputFrameIdx) {
                            //std::cout << " -- popping " << m_nRequestInputFrameIndex-m_nNextExpectedInputFrameIdx << " item(s) from cache" << std::endl;
                            while(m_nRequestInputFrameIndex-m_nNextExpectedInputFrameIdx>0) {
                                m_qoInputFrameCache.pop_front();
                                ++m_nNextExpectedInputFrameIdx;
                            }
                            m_oReqInputFrame = m_qoInputFrameCache.front();
                            m_qoInputFrameCache.pop_front();
                        }
                        else {
                            //std::cout << " -- destroying cache" << std::endl;
                            m_qoInputFrameCache.clear();
                            m_oReqInputFrame = GetInputFrameFromIndex_Internal(m_nRequestInputFrameIndex);
                            m_nNextPrecachedInputFrameIdx = m_nRequestInputFrameIndex+1;
                        }
                    }
                    else {
                        //std::cout << " @ answering request manually, precaching is falling behind" << std::endl;
                        m_oReqInputFrame = GetInputFrameFromIndex_Internal(m_nRequestInputFrameIndex);
                        m_nNextPrecachedInputFrameIdx = m_nRequestInputFrameIndex+1;
                    }
                }
            }
            //else std::cout << " @ answering request using last frame" << std::endl;
            m_nNextExpectedInputFrameIdx = m_nRequestInputFrameIndex+1;
#if PLATFORM_SUPPORTS_CPP11
            m_oInputFrameSyncCondVar.notify_one();
#elif PLATFORM_USES_WIN32API //!PLATFORM_SUPPORTS_CPP11
            WakeConditionVariable(&m_oInputFrameSyncCondVar);
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for precached io support on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
        }
        else {
            CV_DbgAssert((m_nNextPrecachedInputFrameIdx-m_nNextExpectedInputFrameIdx)==m_qoInputFrameCache.size());
            if(m_qoInputFrameCache.size()<PRECACHE_REFILL_THRESHOLD && m_nNextPrecachedInputFrameIdx<m_nTotalNbFrames) {
                //std::cout << " @ filling precache buffer... (" << MAX_NB_PRECACHED_FRAMES-m_qoInputFrameCache.size() << " frames)" << std::endl;
                while(m_qoInputFrameCache.size()<MAX_NB_PRECACHED_FRAMES && m_nNextPrecachedInputFrameIdx<m_nTotalNbFrames)
                    m_qoInputFrameCache.push_back(GetInputFrameFromIndex_Internal(m_nNextPrecachedInputFrameIdx++));
            }
        }
    }
#if !PLATFORM_SUPPORTS_CPP11 && PLATFORM_USES_WIN32API
    LeaveCriticalSection(&m_oInputFrameSyncMutex);
#endif //!PLATFORM_SUPPORTS_CPP11 && PLATFORM_USES_WIN32API
}

void DatasetUtils::SequenceInfo::PrecacheGTFrames() {
    srand((unsigned int)(time(nullptr)*m_nTotalNbFrames*m_sName.size()));
#if PLATFORM_SUPPORTS_CPP11
    std::unique_lock<std::mutex> sync_lock(m_oGTFrameSyncMutex);
#elif PLATFORM_USES_WIN32API //!PLATFORM_SUPPORTS_CPP11
    EnterCriticalSection(&m_oGTFrameSyncMutex);
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for precached io support on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
    const size_t nInitFramesToPrecache = PRECACHE_REFILL_THRESHOLD/2 + rand()%(MAX_NB_PRECACHED_FRAMES/2);
    //std::cout << " @ initializing precaching with " << nInitFramesToPrecache << " frames " << std::endl;
    while(m_qoGTFrameCache.size()<nInitFramesToPrecache && m_nNextPrecachedGTFrameIdx<m_nTotalNbFrames)
        m_qoGTFrameCache.push_back(GetGTFrameFromIndex_Internal(m_nNextPrecachedGTFrameIdx++));
    while(m_bIsPrecaching) {
#if PLATFORM_SUPPORTS_CPP11
        if(m_oGTFrameReqCondVar.wait_for(sync_lock,std::chrono::milliseconds(QUERY_TIMEOUT_MS))!=std::cv_status::timeout) {
#elif PLATFORM_USES_WIN32API //!PLATFORM_SUPPORTS_CPP11
        if(SleepConditionVariableCS(&m_oGTFrameReqCondVar,&m_oGTFrameSyncMutex,QUERY_TIMEOUT_MS)) {
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for precached io support on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
            CV_DbgAssert(m_nRequestGTFrameIndex<m_nTotalNbFrames);
            if(m_nRequestGTFrameIndex!=m_nNextExpectedGTFrameIdx-1) {
                if(!m_qoGTFrameCache.empty() && m_nRequestGTFrameIndex==m_nNextExpectedGTFrameIdx) {
                    m_oReqGTFrame = m_qoGTFrameCache.front();
                    m_qoGTFrameCache.pop_front();
                }
                else {
                    if(!m_qoGTFrameCache.empty()) {
                        //std::cout << " @ answering request manually, out of order (req=" << m_nRequestGTFrameIndex << ", expected=" << m_nNextExpectedGTFrameIdx <<") ";
                        CV_DbgAssert((m_nNextPrecachedGTFrameIdx-m_qoGTFrameCache.size())==m_nNextExpectedGTFrameIdx);
                        if(m_nRequestGTFrameIndex<m_nNextPrecachedGTFrameIdx && m_nRequestGTFrameIndex>m_nNextExpectedGTFrameIdx) {
                            //std::cout << " -- popping " << m_nRequestGTFrameIndex-m_nNextExpectedGTFrameIdx << " item(s) from cache" << std::endl;
                            while(m_nRequestGTFrameIndex-m_nNextExpectedGTFrameIdx>0) {
                                m_qoGTFrameCache.pop_front();
                                ++m_nNextExpectedGTFrameIdx;
                            }
                            m_oReqGTFrame = m_qoGTFrameCache.front();
                            m_qoGTFrameCache.pop_front();
                        }
                        else {
                            //std::cout << " -- destroying cache" << std::endl;
                            m_qoGTFrameCache.clear();
                            m_oReqGTFrame = GetGTFrameFromIndex_Internal(m_nRequestGTFrameIndex);
                            m_nNextPrecachedGTFrameIdx = m_nRequestGTFrameIndex+1;
                        }
                    }
                    else {
                        //std::cout << " @ answering request manually, precaching is falling behind" << std::endl;
                        m_oReqGTFrame = GetGTFrameFromIndex_Internal(m_nRequestGTFrameIndex);
                        m_nNextPrecachedGTFrameIdx = m_nRequestGTFrameIndex+1;
                    }
                }
            }
            //else std::cout << " @ answering request using last frame" << std::endl;
            m_nNextExpectedGTFrameIdx = m_nRequestGTFrameIndex+1;
#if PLATFORM_SUPPORTS_CPP11
            m_oGTFrameSyncCondVar.notify_one();
#elif PLATFORM_USES_WIN32API //!PLATFORM_SUPPORTS_CPP11
            WakeConditionVariable(&m_oGTFrameSyncCondVar);
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for precached io support on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
        }
        else {
            CV_DbgAssert((m_nNextPrecachedGTFrameIdx-m_nNextExpectedGTFrameIdx)==m_qoGTFrameCache.size());
            if(m_qoGTFrameCache.size()<PRECACHE_REFILL_THRESHOLD && m_nNextPrecachedGTFrameIdx<m_nTotalNbFrames) {
                //std::cout << " @ filling precache buffer... (" << MAX_NB_PRECACHED_FRAMES-m_qoGTFrameCache.size() << " frames)" << std::endl;
                while(m_qoGTFrameCache.size()<MAX_NB_PRECACHED_FRAMES && m_nNextPrecachedGTFrameIdx<m_nTotalNbFrames)
                    m_qoGTFrameCache.push_back(GetGTFrameFromIndex_Internal(m_nNextPrecachedGTFrameIdx++));
            }
        }
    }
#if !PLATFORM_SUPPORTS_CPP11 && PLATFORM_USES_WIN32API
    LeaveCriticalSection(&m_oGTFrameSyncMutex);
#endif //!PLATFORM_SUPPORTS_CPP11 && PLATFORM_USES_WIN32API
}

void DatasetUtils::SequenceInfo::StartPrecaching() {
    if(!m_bIsPrecaching) {
        m_bIsPrecaching = true;
#if PLATFORM_SUPPORTS_CPP11
        m_hInputFramePrecacher = std::thread(&DatasetUtils::SequenceInfo::PrecacheInputFrames,this);
        m_hGTFramePrecacher = std::thread(&DatasetUtils::SequenceInfo::PrecacheGTFrames,this);
#elif PLATFORM_USES_WIN32API //!PLATFORM_SUPPORTS_CPP11
        InitializeCriticalSection(&m_oInputFrameSyncMutex);
        InitializeCriticalSection(&m_oGTFrameSyncMutex);
        InitializeConditionVariable(&m_oInputFrameReqCondVar);
        InitializeConditionVariable(&m_oGTFrameReqCondVar);
        InitializeConditionVariable(&m_oInputFrameSyncCondVar);
        InitializeConditionVariable(&m_oGTFrameSyncCondVar);
        m_hInputFramePrecacher = CreateThread(NULL,NULL,&DatasetUtils::SequenceInfo::PrecacheInputFramesEntryPoint,(LPVOID)this,0,NULL);
        m_hGTFramePrecacher = CreateThread(NULL,NULL,&DatasetUtils::SequenceInfo::PrecacheGTFramesEntryPoint,(LPVOID)this,0,NULL);
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for precached io support on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
    }
}

void DatasetUtils::SequenceInfo::StopPrecaching() {
    if(m_bIsPrecaching) {
        m_bIsPrecaching = false;
#if PLATFORM_SUPPORTS_CPP11
        m_hInputFramePrecacher.join();
        m_hGTFramePrecacher.join();
#elif PLATFORM_USES_WIN32API //!PLATFORM_SUPPORTS_CPP11
        //CloseHandle();
        WaitForSingleObject(m_hInputFramePrecacher,INFINITE);
        WaitForSingleObject(m_hGTFramePrecacher,INFINITE);
        CloseHandle(m_hInputFramePrecacher);
        CloseHandle(m_hGTFramePrecacher);
        DeleteCriticalSection(&m_oInputFrameSyncMutex);
        DeleteCriticalSection(&m_oGTFrameSyncMutex);
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for precached io support on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
    }
}

#endif //USE_PRECACHED_IO
