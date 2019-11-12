package com.iotsystem.iotclient;

import java.util.HashMap;

public interface UPnPListener {
    void handleResponse(HashMap<String,String> responseParams);
}
