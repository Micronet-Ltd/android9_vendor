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
import android.graphics.Rect;
import android.util.AttributeSet;

import com.android.launcher3.Insettable;
import com.android.launcher3.Launcher;
import com.android.launcher3.qsb.QsbContainerView;

public class HotseatQsbWidget extends QsbContainerView implements Insettable {

    private final Launcher mLauncher;

    public HotseatQsbWidget(Context context) {
        this(context, null);
    }

    public HotseatQsbWidget(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public HotseatQsbWidget(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        mLauncher = Launcher.getLauncher(context);
    }

    @Override
    public void setInsets(Rect insets) {
        setTranslationY(-insets.bottom);
        setVisibility(mLauncher.getDeviceProfile().isVerticalBarLayout() ? GONE : VISIBLE);

        Rect padding = mLauncher.getDeviceProfile().getHotseatLayoutPadding();
        setPaddingUnchecked(padding.left, 0, padding.right, 0);
    }

    public static class HotseatQsbFragment extends QsbFragment {

        @Override
        public boolean isQsbEnabled() {
            return true;
        }
    }
}
