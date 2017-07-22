import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { HttpModule } from '@angular/http';

import { RaspiCameraRoutingModule } from './raspi-camera-routing.module';
import { ImagePreviewComponent } from './image-preview/image-preview.component';
import { RaspiCameraGphotoService } from './raspi-camera-gphoto.service';
import { PreviewComponent } from './preview/preview.component';

@NgModule({
  imports: [
    CommonModule,
    HttpModule,
    RaspiCameraRoutingModule
  ],
  declarations: [ImagePreviewComponent, PreviewComponent],
  exports: [
    PreviewComponent
  ],
  providers: [
    RaspiCameraGphotoService
  ]
})
export class RaspiCameraModule { }
