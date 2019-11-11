package com.iotsystem.iotclient;

import android.content.Context;
import android.net.wifi.WifiManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.text.format.Formatter;
import android.view.View;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    private TextView mAddressText;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mAddressText = findViewById(R.id.adresses);

        findViewById(R.id.search_button).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                StringBuilder builder = new StringBuilder();
                String[] addr = UPnPDiscovery.discoverDevices(getApplicationContext());
                if(addr != null && addr.length > 0){
                    for(String s : addr){
                        builder.append(s);
                        builder.append('\n');
                    }
                }
                mAddressText.setText(builder.toString());
            }
        });
    }
}
