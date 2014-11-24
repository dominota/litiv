#pragma once

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <unordered_map>
#include <deque>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include "PlatformUtils.h"

#define USE_AVERAGE_METRICS      1
#define USE_PRECACHED_IO         0 // @@@@@@ reimpl caching with solid malloc'd arrays
#if USE_PRECACHED_IO
#define MAX_NB_PRECACHED_FRAMES   100
#define PRECACHE_REFILL_THRESHOLD (MAX_NB_PRECACHED_FRAMES/4)
#define REQUEST_TIMEOUT_MS        1
#define QUERY_TIMEOUT_MS          10
#endif //USE_PRECACHED_IO

namespace DatasetUtils {
    class MetricsCalculator;
    class SequenceInfo;
    class CategoryInfo;

    // as defined in the 2012 CDNet scripts/dataset
    const uchar g_nCDnetPositive = 255;
    const uchar g_nCDnetNegative = 0;
    const uchar g_nCDnetOutOfScope = 85;
    const uchar g_nCDnetUnknown = 170;
    const uchar g_nCDnetShadow = 50;

    double CalcMetric_FMeasure(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN);
    double CalcMetric_Recall(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN);
    double CalcMetric_Precision(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN);
    double CalcMetric_Specificity(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN);
    double CalcMetric_FalsePositiveRate(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN);
    double CalcMetric_FalseNegativeRate(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN);
    double CalcMetric_PercentBadClassifs(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN);
    double CalcMetric_MatthewsCorrCoeff(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN);

    cv::Mat ReadResult( const std::string& sResultsPath, const std::string& sCatName, const std::string& sSeqName,
                               const std::string& sResultPrefix, size_t framenum, const std::string& sResultSuffix);
    void WriteResult( const std::string& sResultsPath, const std::string& sCatName, const std::string& sSeqName,
                             const std::string& sResultPrefix, size_t framenum, const std::string& sResultSuffix,
                             const cv::Mat& res, const std::vector<int>& vnComprParams);
    void WriteOnImage(cv::Mat& oImg, const std::string& sText, bool bBottom=false);
    void WriteMetrics(const std::string sResultsFileName, const SequenceInfo* pSeq);
    void WriteMetrics(const std::string sResultsFileName, CategoryInfo* pCat);
    void WriteMetrics(const std::string sResultsFileName, std::vector<CategoryInfo*>& vpCat, double dTotalFPS);
    void CalcMetricsFromResult(const cv::Mat& oSegmResFrame, const cv::Mat& oGTFrame, const cv::Mat& oROI,
                                      uint64_t& nTP, uint64_t& nTN, uint64_t& nFP, uint64_t& nFN, uint64_t& nSE);
    cv::Mat GetColoredSegmFrameFromResult(const cv::Mat& oSegmResFrame, const cv::Mat& oGTFrame, const cv::Mat& oROI);

    struct CommonMetrics {
        double dRecall;
        double dSpecficity;
        double dFPR;
        double dFNR;
        double dPBC;
        double dPrecision;
        double dFMeasure;
        double dMCC;
        double dFPS;
    };

    class MetricsCalculator {
    public:
        MetricsCalculator(uint64_t nTP, uint64_t nTN, uint64_t nFP, uint64_t nFN, uint64_t nSE);
        MetricsCalculator(const SequenceInfo* pSeq);
        MetricsCalculator(const CategoryInfo* pCat, bool bAverage=false);
        MetricsCalculator(const std::vector<CategoryInfo*>& vpCat, bool bAverage=false);
        const CommonMetrics m_oMetrics;
        const bool m_bAveraged;
    };

    enum eAvailableDatasetsID {
        eDataset_CDnet=0,
        eDataset_Wallflower,
        eDataset_PETS2001_D3TC1,
        eDataset_LITIV_Registr01,
        eDataset_GenericSegmentationTest,
        eDatasetsCount
    };

    class CategoryInfo {
    public:
        CategoryInfo(const std::string& sName, const std::string& sDirectoryPath,
                     DatasetUtils::eAvailableDatasetsID eDatasetID,
                     const char* asGrayscaleDirNameTokens[]=nullptr);
        ~CategoryInfo();
        const std::string m_sName;
        const eAvailableDatasetsID m_eDatasetID;
        std::vector<SequenceInfo*> m_vpSequences;
        uint64_t nTP, nTN, nFP, nFN, nSE;
        double m_dAvgFPS;
        static inline bool compare(const CategoryInfo* i, const CategoryInfo* j) {return PlatformUtils::compare_lowercase(i->m_sName,j->m_sName);}
    private:
#if PLATFORM_SUPPORTS_CPP11
        CategoryInfo& operator=(const CategoryInfo&) = delete;
        CategoryInfo(const CategoryInfo&) = delete;
#else //!PLATFORM_SUPPORTS_CPP11
        CategoryInfo& operator=(const CategoryInfo&);
        CategoryInfo(const CategoryInfo&);
#endif //!PLATFORM_SUPPORTS_CPP11
    };

    class SequenceInfo {
    public:
        SequenceInfo(const std::string& sName, const std::string& sPath, CategoryInfo* pParent, bool bForceGrayscale=false);
        ~SequenceInfo();
        const cv::Mat& GetInputFrameFromIndex(size_t nFrameIdx);
        const cv::Mat& GetGTFrameFromIndex(size_t nFrameIdx);
        size_t GetNbInputFrames() const;
        size_t GetNbGTFrames() const;
        cv::Size GetFrameSize() const;
        const cv::Mat& GetSequenceROI() const;
        std::vector<cv::KeyPoint> GetKeyPointsFromROI() const;
        void ValidateKeyPoints(std::vector<cv::KeyPoint>& voKPs) const;
        const std::string m_sName;
        const std::string m_sPath;
        const eAvailableDatasetsID m_eDatasetID;
        uint64_t nTP, nTN, nFP, nFN, nSE;
        double m_dAvgFPS;
        double m_dExpectedLoad;
        double m_dExpectedROILoad;
        CategoryInfo* m_pParent;
        inline cv::Size GetSize() {return m_oSize;}
        static inline bool compare(const SequenceInfo* i, const SequenceInfo* j) {return PlatformUtils::compare_lowercase(i->m_sName,j->m_sName);}
#if USE_PRECACHED_IO
        void StartPrecaching();
        void StopPrecaching();
    private:
        void PrecacheInputFrames();
        void PrecacheGTFrames();
#if PLATFORM_SUPPORTS_CPP11
        std::thread m_hInputFramePrecacher,m_hGTFramePrecacher;
        std::mutex m_oInputFrameSyncMutex,m_oGTFrameSyncMutex;
        std::condition_variable m_oInputFrameReqCondVar,m_oGTFrameReqCondVar;
        std::condition_variable m_oInputFrameSyncCondVar,m_oGTFrameSyncCondVar;
#elif PLATFORM_USES_WIN32API //&& !PLATFORM_SUPPORTS_CPP11
        HANDLE m_hInputFramePrecacher,m_hGTFramePrecacher;
        static inline DWORD WINAPI PrecacheInputFramesEntryPoint(LPVOID lpParam) {try{((SequenceInfo*)lpParam)->PrecacheInputFrames();}catch(...){return-1;}return 0;}
        static inline DWORD WINAPI PrecacheGTFramesEntryPoint(LPVOID lpParam) {try{((SequenceInfo*)lpParam)->PrecacheGTFrames();}catch(...){return-1;} return 0;}
        CRITICAL_SECTION m_oInputFrameSyncMutex,m_oGTFrameSyncMutex;
        CONDITION_VARIABLE m_oInputFrameReqCondVar,m_oGTFrameReqCondVar;
        CONDITION_VARIABLE m_oInputFrameSyncCondVar,m_oGTFrameSyncCondVar;
#else //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
#error "Missing implementation for semaphores on this platform."
#endif //!PLATFORM_USES_WIN32API && !PLATFORM_SUPPORTS_CPP11
        bool m_bIsPrecaching;
        size_t m_nRequestInputFrameIndex,m_nRequestGTFrameIndex;
        std::deque<cv::Mat> m_qoInputFrameCache,m_qoGTFrameCache;
        size_t m_nNextExpectedInputFrameIdx,m_nNextExpectedGTFrameIdx;
        size_t m_nNextPrecachedInputFrameIdx,m_nNextPrecachedGTFrameIdx;
        cv::Mat m_oReqInputFrame,m_oReqGTFrame;
#else //!USE_PRECACHED_IO
    private:
        size_t m_nLastReqInputFrameIndex,m_nLastReqGTFrameIndex;
        cv::Mat m_oLastReqInputFrame,m_oLastReqGTFrame;
#endif //!USE_PRECACHED_IO
        std::vector<std::string> m_vsInputFramePaths;
        std::vector<std::string> m_vsGTFramePaths;
        cv::VideoCapture m_voVideoReader;
        size_t m_nNextExpectedVideoReaderFrameIdx;
        size_t m_nTotalNbFrames;
        cv::Mat m_oROI;
        cv::Size m_oSize;
        const int m_nIMReadInputFlags;
        std::unordered_map<size_t,size_t> m_mTestGTIndexes;
        cv::Mat GetInputFrameFromIndex_Internal(size_t nFrameIdx);
        cv::Mat GetGTFrameFromIndex_Internal(size_t nFrameIdx);
#if PLATFORM_SUPPORTS_CPP11
        SequenceInfo& operator=(const SequenceInfo&) = delete;
        SequenceInfo(const CategoryInfo&) = delete;
#else //!PLATFORM_SUPPORTS_CPP11
        SequenceInfo& operator=(const SequenceInfo&);
        SequenceInfo(const SequenceInfo&);
#endif //!PLATFORM_SUPPORTS_CPP11
    };
}; //namespace DatasetUtils
