import { BrowserModule } from '@angular/platform-browser';
import { NgModule } from '@angular/core';
import { HttpModule } from '@angular/http';

import { AppComponent } from './app.component';
import { RaspiCameraModule } from './raspi-camera/raspi-camera.module';

@NgModule({
  declarations: [
    AppComponent
  ],
  imports: [
    BrowserModule,
    HttpModule,
    RaspiCameraModule
  ],
  providers: [],
  bootstrap: [AppComponent]
})
export class AppModule { }
