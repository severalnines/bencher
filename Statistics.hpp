#ifndef statistics_hpp
#define statistics_hpp


struct ThreadStat
{
  uint max;
  uint min;
  uint count;
  double var;
  double average;
  double sum;
};

ThreadStat::init()
{
  max=count=0;
  min=UINT_MAX;
  var=average=sum=0;
}


class Statistics
{
public:
  Statistics(uint threads, uint samples)
  addSamples(uint thread, uint time_tx);
private:
  int m_maxThreads;
  ThreadStat * m_threadStat;
  
};

Statistics::Statistics(int threads, int samples)
{
  m_maxThreads=threads;
  m_threadState = new ThreadStat[m_maxThreads];
  for (i=0; i<m_maxThreads;i++)
    {
      m_threadStat[i].init();
    }

}

Statistics::addSample(uint thread, uint tx_time)
{
  if(thread < 0  || thread > m_maxThreads)
    exit(0);
  m_threadStat[i].count++;
  m_threadStat[i].sum+=(double)tx_time;
  if(tx_time<m_threadStat[i].min)
    m_threadStat[i].min=tx_time;
  if(tx_time>m_threadStat[i].max)
    m_threadStat[i].max=tx_time;
  
  m_threadStat[i].average= m_threadStat[i].sum/(double)m_threadStat[i].count;
  

  

}

#endif
