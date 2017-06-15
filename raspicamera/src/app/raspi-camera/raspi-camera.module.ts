import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { RaspiCameraRoutingModule } from './raspi-camera-routing.module';
import { ImagePreviewComponent } from './image-preview/image-preview.component';

@NgModule({
  imports: [
    CommonModule,
    RaspiCameraRoutingModule
  ],
  declarations: [ImagePreviewComponent]
})
export class RaspiCameraModule { }
