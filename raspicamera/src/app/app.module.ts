import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';

import { AppComponent } from './app.component';
import { RaspiCameraModule } from './raspi-camera/raspi-camera.module';

@NgModule({
  declarations: [
    AppComponent
  ],
  imports: [
    BrowserModule,
    RaspiCameraModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
