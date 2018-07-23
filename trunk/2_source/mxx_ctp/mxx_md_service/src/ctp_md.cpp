
CCtpConnection::CCtpConnection()
{
  m_ctp_status=CTP_SS_INIT;
  m_md_api=NULL;
  m_md_spi=NULL;
}

CCtpConnection::~CCtpConnection()
{
  if(NULL==m_md_api)
  {
      m_md_api->Release();
      m_md_api=NULL;
  }
  
  if(NULL==m_md_spi)
  {
    delete m_md_spi;
    m_md_spi=NULL;
  }
}