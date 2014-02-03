//
//  FirstViewController.h
//  IceStar
//
//  Created by Andreas Müller on 31.01.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DeviceListController.h"
#import "BinningController.h"

@interface FirstViewController : UIViewController<UIPopoverControllerDelegate,UITableViewDelegate> {
    IBOutlet UIButton   *cameraButton;
    IBOutlet UIButton   *ccdButton;
    IBOutlet UIButton   *binningButton;
    IBOutlet UITextField    *exposureField;
    IBOutlet UISwitch   *shutterSwitch;
    
    IBOutlet UISwitch   *coolerSwitch;
    IBOutlet UISlider   *temperatureSlider;
    IBOutlet UILabel    *temperatureLabel;
    IBOutlet UILabel    *actualtemperatureLabel;
    
    IBOutlet UIButton   *captureButton;
    
    IBOutlet UIScrollView   *scrollView;
    UIImageView             *imageView;
    
    NSDate  *exposureStart;
    IBOutlet UIProgressView *progressView;
    
    NSTimer *timer; // for temperature updates
    NSTimer *progresstimer; // timer for progress updates
    NSTimer *exposuretimer; // for timing the end of the exposure
    
    UIPopoverController *popover;
    
    snowstarCameraPrx   *camera;
    snowstarCcdPrx  *ccd;
    snowstarCcdInfo *ccdinfo;
    snowstarCoolerPrx   *cooler;
    snowstarExposure    *exposure;
    
    BOOL    allowTemperatureUpdate;
}

- (IBAction)cameraselection:(id)sender;
- (IBAction)binningselection:(id)sender;

- (void)selectCamera: (NSString *)cameraname;
- (void)selectCcd: (int)ccdid;
- (void)selectBinning: (snowstarBinningMode *)binning;

- (IBAction)temperatureChanged: (id)sender;

- (IBAction)toggleExposure: (id)sender;
- (void)timerTick: (NSTimer *)timer;
- (void)retrieveImage: (NSTimer *)exposuretimer;
- (void)updateProgress: (NSTimer *)progresstimer;
- (IBAction)exposuretimeChanged: (id)sender;

@end
