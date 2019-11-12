package com.iotsystem.iotclient;

import android.content.Context;
import android.net.wifi.WifiManager;
import android.os.AsyncTask;
import android.util.Log;

import java.io.IOException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.util.HashMap;

/**
 * @author Bernd Verst(@berndverst)
 */
public class UPnPDiscovery extends AsyncTask<Context, Void, String[]> {

    private final String SSDP_ADDR = "239.255.255.250";
    private final int SSDP_PORT = 1900;
    private final int CONNECTION_TIMEOUT_MS = 2000;
    private final int RESPONSE_LENGTH = 512;
    private final String QUERY = "M-SEARCH * HTTP/1.1\r\n" +
            "HOST: 239.255.255.250:1900\r\n" +
            "MAN: \"ssdp:discover\"\r\n" +
            "MX: 2\r\n" +
            "ST: IOTGATEWAY\r\n" +
            "\r\n";
    private UPnPListener mListener = null;

    public UPnPDiscovery(UPnPListener uPnPListener) {
        mListener = uPnPListener;
    }

    /* Send SSDP Request and wait for the first gateway response.*/
    @Override
    protected String[] doInBackground(Context... contexts) {
        String[] response = null;
        WifiManager wifi = (WifiManager) contexts[0].getApplicationContext()
                .getSystemService(Context.WIFI_SERVICE);
        if (wifi != null) {
            WifiManager.MulticastLock lock = wifi.createMulticastLock("The Lock");
            if (!lock.isHeld())
                lock.acquire();
            DatagramSocket socket = null;
            try {
                InetAddress group = InetAddress.getByName(SSDP_ADDR);
                socket = new DatagramSocket(SSDP_PORT);
                socket.setReuseAddress(true);
                socket.setBroadcast(true);
                DatagramPacket datagramRequest = new DatagramPacket(QUERY.getBytes(), QUERY.length(),
                        group, SSDP_PORT);
                socket.send(datagramRequest);
                socket.setSoTimeout(CONNECTION_TIMEOUT_MS);

                DatagramPacket datagramResponse = new DatagramPacket(new byte[RESPONSE_LENGTH],
                        RESPONSE_LENGTH);
                socket.receive(datagramResponse);
                String data = new String(datagramResponse.getData(), 0,
                        datagramResponse.getLength());
                response = data.split("\r\n");
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                if (socket != null)
                    socket.close();
            }
            if (lock.isHeld())
                lock.release();
        }
        return response;
    }

    @Override
    protected void onPostExecute(String[] responseLines) {
        if (mListener != null) {
            HashMap<String, String> params = new HashMap<>();
            if (responseLines != null) {
                params.put("STATUS", responseLines[0]);
                for (int index = 1; index < responseLines.length; index++) {
                    if (!responseLines[index].isEmpty()) {
                        String key = null;
                        String value = null;
                        try {
                            key = responseLines[index].split(":", 2)[0];
                            value = responseLines[index].split(":", 2)[1];
                        } catch (IndexOutOfBoundsException e) {
                            e.printStackTrace();
                        } finally {
                            if (key != null && value != null) {
                                params.put(key, value);
                            }
                        }
                    }
                }
            }
            mListener.handleResponse(params);
        }
    }
}