#include "wbsignaldetectmodel.h"

WBSignalDetectModel::WBSignalDetectModel(QObject *parent) : QAbstractTableModel(parent)
{

}

int WBSignalDetectModel::FindSignal(float *FFtin, int InStep, int length, int Freqency, int BandWidth)
{
    int AvgCnt = 0;
    Ipp32f * FFtAvg = ippsMalloc_32f(length);
    //fprint32f("FFtIn.txt", FFtin, length);
    //获取活滑动Step个点的平均FFt波形
    for (int i = 0; i < length; i++)
    {
        AvgCnt = (i - InStep / 2) >= 0 ? 32 : (i + InStep / 2);
        if (i <= InStep / 2)
        {
            AvgCnt = i + InStep / 2;
            ippsMean_32f(FFtin, AvgCnt, &FFtAvg[i], ippAlgHintNone);
        }
        else if((length - i) <= InStep)
        {
            AvgCnt = length - i;
            ippsMean_32f(&FFtin[i], AvgCnt, &FFtAvg[i], ippAlgHintNone);
        }
        else
        {
            ippsMean_32f(&FFtin[i], InStep, &FFtAvg[i], ippAlgHintNone);
        }
    }
    //TODO: 状态错误时的处理方法
    //记录本次检测到的大功率信号 list
    int ret = 1;
    findPeakCyclically(FFtAvg, length, Freqency, BandWidth);
    if(!m_lstSignalInfo.isEmpty()){
        ret = 0;
    }

    //与已积累下来的保持存活信号进行比较 map

    //对已完成的信号进行保存 list

    ippsFree(FFtAvg);
    return ret;
}

int WBSignalDetectModel::rowCount(const QModelIndex &parent) const
{

}

int WBSignalDetectModel::columnCount(const QModelIndex &parent) const
{

}

QVariant WBSignalDetectModel::data(const QModelIndex &index, int role) const
{

}

int WBSignalDetectModel::SampleDownFromInBuf(short *inBuf, int len, short *outBuf, int factor){
    int outBufLength = 0;
    int phase = 0;
    ippsSampleDown_16s(inBuf, len, outBuf,&outBufLength,factor, &phase);
    return outBufLength;
}

bool WBSignalDetectModel::findPeakIteratively(Ipp32f * FFtAvg, int length, int Freqency, int BandWidth)
{
    //从左向右查找第一个峰值的位置
    Ipp32f SignalPower = 0;
    Ipp32f NoisePower = 0;
    Ipp32f NoiseLeftPower = 0;
    Ipp32f NoiseRightPower = 0;
    Ipp32f FFtMax = m_fThreshold;
    int MaxAddr = 0;
    int LeftAddr = 0;
    int RightAddr = length;

    //LZMK:对原有逻辑进行改造，原有逻辑为根据对应区间获取最大值最高点作为信号peak；
    //当前采用门限进行限制，只要存在某一点比前后两个点的幅度大的情况，即可认为是一个尖峰
    for(int index = 1; index < length - 1; ++index){
        if(FFtAvg[index] <= m_fThreshold){
            continue;
        }
        if(FFtAvg[index + 1] >= FFtAvg[index] || FFtAvg[index] <= FFtAvg[index - 1]){
            continue;
        }
        FFtMax = FFtAvg[index];
        MaxAddr = index;
        break;
    }

    //找不到最大点了就可以退出了
    if(MaxAddr == 0){
        return true;
    }

    //TODO: 是否需要处理两个波峰过于接近，导致没能在右侧找到6dB边界的情况？
    //处理出现未找到6dB右边界的情况且包络走势出现上扬的趋势时直接作为右边界
    for(int index = MaxAddr; index < length - 1; ++index){
        RightAddr = index;
        if(FFtAvg[index + 1] > FFtAvg[index]){
            break;
        }
        if(FFtMax - FFtAvg[index] > 6){
            break;
        }
    }

    //按照最高峰位置反向搜索信号 寻找左边界
    for (int index = MaxAddr; index > 0; index--)
    {
        LeftAddr = index;//获取左边界
        if(FFtAvg[index - 1] > FFtAvg[index]){
            break;
        }
        if (FFtMax - FFtAvg[index] > 6)//6dB右边界
        {
            break;
        }
    }

    //计算信号属性
    SignalInfoStr currentSignalInfo;
    currentSignalInfo.Bound = (RightAddr - LeftAddr)*(float)((float)BandWidth / (float)length);
    currentSignalInfo.CentFreq = Freqency + ((RightAddr+ LeftAddr)/2 - length / 2)*(float)((float)BandWidth / (float)length);
    //获取信号平均功率
    ippsMean_32f(&FFtAvg[LeftAddr], RightAddr - LeftAddr + 1, &SignalPower, ippAlgHintFast);
    //获取噪声平均功率
    ippsMean_32f(FFtAvg, LeftAddr, &NoiseLeftPower, ippAlgHintFast);
    ippsMean_32f(&FFtAvg[RightAddr], length - RightAddr, &NoiseRightPower, ippAlgHintFast);
    NoisePower = (NoiseRightPower + NoiseLeftPower) / 2;
    currentSignalInfo.Snr = SignalPower - NoisePower;
    m_lstSignalInfo.append(currentSignalInfo);

    //后续处理时全带宽也会缩短
    int notDealedBandWidth = BandWidth - currentSignalInfo.Bound;
    int notDealedBandCenterFreq = Freqency + RightAddr * (float)BandWidth / (float)length;
    return findPeakIteratively(FFtAvg + (RightAddr + 1), length - (RightAddr + 1), notDealedBandCenterFreq, notDealedBandWidth);
}

bool WBSignalDetectModel::findPeakCyclically(Ipp32f * FFtAvg, int length, int Freqency, int BandWidth)
{
    //从左向右查找第一个峰值的位置
    Ipp32f SignalPower = 0;
    Ipp32f NoisePower = 0;
    Ipp32f NoiseLeftPower = 0;
    Ipp32f NoiseRightPower = 0;
    //LZMK:对原有逻辑进行改造，原有逻辑为根据对应区间获取最大值最高点作为信号peak；
    //当前采用门限进行限制，只要存在某一点比前后两个点的幅度大的情况，即可认为是一个尖峰
    for(int totalIndex = 0; totalIndex < length;){
        Ipp32f FFtMax = m_fThreshold;
        int MaxAddr = 0;
        int LeftAddr = 0;
        int RightAddr = length;
        for(int index = totalIndex + 1; index < length - 1; ++index){
            if(FFtAvg[index] <= m_fThreshold){
                continue;
            }
            if(FFtAvg[index + 1] >= FFtAvg[index] || FFtAvg[index] <= FFtAvg[index - 1]){
                continue;
            }
            FFtMax = FFtAvg[index];
            MaxAddr = index;
            break;
        }

        //找不到最大点了就可以退出了
        if(MaxAddr == 0){
            return true;
        }

        //TODO: 是否需要处理两个波峰过于接近，导致没能在右侧找到6dB边界的情况？
        //处理出现未找到6dB右边界的情况且包络走势出现上扬的趋势时直接作为右边界
        for(int index = MaxAddr; index < length - 1; ++index){
            RightAddr = index;
            if(FFtAvg[index + 1] > FFtAvg[index]){
                break;
            }
            if(FFtMax - FFtAvg[index] > 6){
                break;
            }
        }

        //按照最高峰位置反向搜索信号 寻找左边界
        for (int index = MaxAddr; index > totalIndex; index--)
        {
            LeftAddr = index;//获取左边界
            if(FFtAvg[index - 1] > FFtAvg[index]){
                break;
            }
            if (FFtMax - FFtAvg[index] > 6)//6dB右边界
            {
                break;
            }
        }

        //计算信号属性
        SignalInfoStr currentSignalInfo;
        currentSignalInfo.Bound = (RightAddr - LeftAddr)*(float)((float)BandWidth / (float)length);
        currentSignalInfo.CentFreq = Freqency + ((RightAddr+ LeftAddr)/2 - length / 2)*(float)((float)BandWidth / (float)length);
        //获取信号平均功率
        ippsMean_32f(&FFtAvg[LeftAddr], RightAddr - LeftAddr + 1, &SignalPower, ippAlgHintFast);
        //获取噪声平均功率
        ippsMean_32f(FFtAvg, LeftAddr, &NoiseLeftPower, ippAlgHintFast);
        ippsMean_32f(&FFtAvg[RightAddr], length - RightAddr, &NoiseRightPower, ippAlgHintFast);
        NoisePower = (NoiseRightPower + NoiseLeftPower) / 2;
        currentSignalInfo.Snr = SignalPower - NoisePower;
        m_lstSignalInfo.append(currentSignalInfo);
        //更新次回处理起点
        totalIndex = RightAddr + 1;
    }
    return true;
}
