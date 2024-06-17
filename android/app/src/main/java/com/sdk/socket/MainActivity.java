package com.sdk.socket;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Button;

import com.sdk.socket.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'socket' library on application startup.
    static {
        System.loadLibrary("SocketAndroid");
    }

    private ActivityMainBinding binding;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Example of a call to a native method
        Button reqBtn = binding.request;

        reqBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // Code to execute when the button is clicked
                String ipAddress = binding.ipaddress.getText().toString();
                String portNumber = binding.portNumber.getText().toString();
                String reqMessage = binding.requestMessage.getText().toString();
                EditText result = binding.result;
                result.setText(SendRequest(ipAddress, Integer.parseInt(portNumber), reqMessage));
            }
        });
    }

    /**
     * A native method that is implemented by the 'socket' native library,
     * which is packaged with this application.
     */
    public native String SendRequest(String ipAddress, int portNumber, String reqMsg);
}