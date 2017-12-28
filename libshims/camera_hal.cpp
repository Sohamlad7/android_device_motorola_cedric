/*
 * Copyright (C) 2016 The CyanogenMod Project
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

#include <EGL/egl.h>
#include <EGL/eglext.h>

/* TODO: Actually provide implementations for these function! */

extern "C" void _ZN7android20DisplayEventReceiverC1Ev() {}

EGLAPI const char* eglQueryStringImplementationANDROID(EGLDisplay dpy, EGLint name);

extern "C" void _Z35eglQueryStringImplementationANDROIDPvi(EGLDisplay dpy, EGLint name){
    eglQueryStringImplementationANDROID(dpy, name);
}
