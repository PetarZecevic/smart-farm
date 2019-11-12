package com.iotsystem.iotclient;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.TextView;

import java.util.HashMap;

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
                new UPnPDiscovery(new UPnPListener() {
                    @Override
                    public void handleResponse(HashMap<String, String> responseParams) {
                        if(responseParams.containsKey("LOCATION")){
                            mAddressText.setText(responseParams.get("LOCATION"));
                        } else {
                            mAddressText.setText("NOT FOUND");
                        }
                    }
                }).execute(getApplicationContext());
            }
        });
    }
}
