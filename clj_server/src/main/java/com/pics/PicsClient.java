package com.pics;

public final class PicsClient {
  protected long _handle;

  static 
  {
    System.loadLibrary("picsjni");
  }

  public PicsClient() 
  {
    _handle = 0;
  }

  public void connect() throws Exception
  {
    long handle = _connect();
    if (handle == 0)
    {
      throw new Exception("connection failed");
    }

    _handle = handle;
  }  

  public void disconnect() 
  {
    if (_handle != 0)
    {
      _disconnect(_handle);
      _handle = 0;
    }
  }

  public String[] search(Object[] desc) throws ServiceNotAvailableException
  {
    if (_isSearching(_handle))
    {
      throw new ServiceNotAvailableException();
    }

    return _search(_handle, desc);
  }

  private native long _connect();
  private native void _disconnect(long handle);

  // thread-safe
  private native boolean _isSearching(long handle);
  private native String[] _search(long handle, Object[] desc);
}


