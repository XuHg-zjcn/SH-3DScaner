/*
 * Copyright 2020 Xu Ruijun
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//some code copy from https://github.com/googlearchive/android-Camera2Basic
package com.example.sh3dscaner;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    private Button Button_camera2api;
    private Button Button_opencv;
    private Intent intent;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        //Toast.makeText(MainActivity.this, "应用启动,测试消息", Toast.LENGTH_LONG).show();
        /*if (null == savedInstanceState) {
            getSupportFragmentManager().beginTransaction()
                    .replace(R.id.container, Camera2BasicFragment.newInstance())
                    .commit();
        }*/
        Button_camera2api = findViewById(R.id.button_camera2api);
        Button_camera2api.setOnClickListener(this);
        Button_opencv = findViewById(R.id.button2_opencv);
        Button_opencv.setOnClickListener(this);
    }

    @Override
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.button_camera2api:
                intent = new Intent(MainActivity.this, Camera2BasicFragment.class);
                startActivity(intent);
                break;
            case R.id.button2_opencv:
                intent = new Intent(MainActivity.this, OpenCVTest.class);
                startActivity(intent);
                break;
            default:
                break;
        }
    }

}
