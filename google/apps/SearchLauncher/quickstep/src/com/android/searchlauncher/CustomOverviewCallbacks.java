/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.searchlauncher;

import android.content.Context;

import com.android.launcher3.LauncherAppState;
import com.android.quickstep.OverviewCallbacks;

import com.google.android.libraries.gsa.launcherclient.LauncherClient;

@SuppressWarnings("unused")
public class CustomOverviewCallbacks extends OverviewCallbacks {

    public CustomOverviewCallbacks(Context context) { }

    @Override
    public void onInitOverviewTransition() {
        SearchLauncher activity = getActvity();
        if (activity != null) {
            activity.getCallbacks().deferCallbacksUntilNextResumeOrStop();
        }
    }

    @Override
    public void closeAllWindows() {
        SearchLauncher activity = getActvity();
        if (activity == null) {
            return;
        }
        LauncherClient client = activity.getCallbacks().getLauncherClient();
        if (client == null) {
            return;
        }
        if (!activity.isStarted() || activity.isForceInvisible()) {
            client.hideOverlay(false /* animate */);
        } else {
            client.hideOverlay(150);
        }
    }

    private SearchLauncher getActvity() {
        LauncherAppState app = LauncherAppState.getInstanceNoCreate();
        if (app == null) {
            return null;
        }
        return (SearchLauncher) app.getModel().getCallback();
    }
}
