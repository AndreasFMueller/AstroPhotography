//
//  FirstViewController.m
//  IceStar
//
//  Created by Andreas Müller on 31.01.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import <Ice/Ice.h>
#import <device.h>
#import <Foundation/NSAutoreleasePool.h>
#import "FirstViewController.h"
#import "AppDelegate.h"
#import "CcdSelectionController.h"
#include <math.h>
#import "image.h"

@interface FirstViewController ()

@end

@implementation FirstViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    // add the image View
    imageView = [[UIImageView alloc] init];
    [scrollView addSubview: imageView];
    NSLog(@"scrollview size: %f x %f",scrollView.frame.size.width, scrollView.frame.size.height);
	// Do any additional setup after loading the view, typically from a nib.
    exposure = [[snowstarExposure alloc] init];
    exposure.mode = [[snowstarBinningMode alloc] init];
    exposure.mode.x = 1;
    exposure.mode.y = 1;
    exposure.frame = [[snowstarImageRectangle alloc] init];
    exposure.frame.size = [[snowstarImageSize alloc] init];
    exposure.frame.size.width = 0;
    exposure.frame.size.height = 0;
    exposure.frame.origin = [[snowstarImagePoint alloc] init];
    exposure.frame.origin.x = 0;
    exposure.frame.origin.y = 0;
    exposure.gain = 1;
    exposure.limit = 1000000;
    timer = [NSTimer timerWithTimeInterval: 2.0 target: self selector: @selector(timerTick:) userInfo: nil repeats: YES];
    [[NSRunLoop currentRunLoop] addTimer: timer forMode: NSDefaultRunLoopMode];
    NSLog(@"timer created");
    allowTemperatureUpdate = YES;
    [timer fire];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)cameraselection:(id)sender {
    UIButton    *button = (UIButton *)sender;
    NSLog(@"create a camera selection");
    DeviceListController    *devlist = [[DeviceListController alloc] initWithDeviceType: snowstarDevCAMERA];
    devlist.tableView.delegate = self;
    popover = [[UIPopoverController alloc] initWithContentViewController: devlist];
    popover.delegate = self;
    [popover presentPopoverFromRect: CGRectMake(button.frame.size.width / 2, button.frame.size.height / 2, 1, 1) inView: sender permittedArrowDirections:UIPopoverArrowDirectionAny animated: YES];
}

- (IBAction)ccdSelection: (id)sender {
    if (nil == camera) {
        return;
    }
    UIButton    *button = (UIButton *)sender;
    CcdSelectionController  *selection = [[CcdSelectionController alloc] initWithCamera: camera];
    selection.tableView.delegate = self;
    popover = [[UIPopoverController alloc] initWithContentViewController: selection];
    popover.delegate = self;
    [popover presentPopoverFromRect: CGRectMake(button.frame.size.width / 2, button.frame.size.height / 2, 1, 1) inView: sender permittedArrowDirections:UIPopoverArrowDirectionAny animated: YES];
}

- (IBAction)binningselection:(id)sender {
    if (nil == ccdinfo) {
        return;
    }
    UIButton    *button = (UIButton *)sender;
    NSLog(@"create a binning selection with %d selections", [ccdinfo.binningmodes count]);
    BinningController    *list = [[BinningController alloc] initWithBinningSet: ccdinfo.binningmodes];
    list.tableView.delegate = self;
    popover = [[UIPopoverController alloc] initWithContentViewController: list];
    popover.delegate = self;
    [popover presentPopoverFromRect: CGRectMake(button.frame.size.width / 2, button.frame.size.height / 2, 1, 1) inView: sender permittedArrowDirections:UIPopoverArrowDirectionAny animated: YES];
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    UIViewController    *content = popover.contentViewController;
    // what to do when the camera is selected
    if ([content isMemberOfClass: [DeviceListController class]]) {
        DeviceListController *devlist = (DeviceListController *)popover.contentViewController;
        NSString    *cameraname = [devlist nameAtIndex: indexPath.row];
        [popover dismissPopoverAnimated:YES];
        
        // select the camera
        [self selectCamera: cameraname];
        return;
    }
    if ([content isMemberOfClass: [CcdSelectionController class]]) {
        [popover dismissPopoverAnimated:YES];
        return;
    }
    if ([content isMemberOfClass: [BinningController class]]) {
        NSLog(@"select binning mode %d", indexPath.row);
        [self selectBinning: [ccdinfo.binningmodes objectAtIndex: indexPath.row]];
        [popover dismissPopoverAnimated:YES];
        return;
    }
}

- (void)selectCamera: (NSString *)cameraname {
    [cameraButton setTitle: [cameraname stringByRemovingPercentEncoding] forState: UIControlStateNormal];
    [cameraButton setTitle: [cameraname stringByRemovingPercentEncoding] forState: UIControlStateSelected];

    // reset the binning mode to the default
    [binningButton setTitle: @"1x1" forState: UIControlStateNormal];
    [binningButton setTitle: @"1x1" forState: UIControlStateNormal];
    exposure.mode.x = 1;
    exposure.mode.y = 1;

    // get an instance of the camera
    AppDelegate *delegate = [UIApplication sharedApplication].delegate;
    id<snowstarDevicesPrx>  devices = delegate.connection.devices;
    camera = [devices getCamera: cameraname];
    
    // find out how many ccds there are, and select an appropriate one
    if ([camera nCcds] == 1) {
        [self selectCcd: 0];
        [ccdButton setEnabled: false];
        return;
    }
    ccd = nil;
    ccdinfo = nil;
    [ccdButton setEnabled: true];
    [ccdButton setTitle: @"Select" forState: UIControlStateNormal];
    [ccdButton setTitle: @"Select" forState: UIControlStateSelected];
    NSLog(@"camera has %d ccds", [camera nCcds]);
    
}

- (void)updateCooler {
    if (nil == cooler) {
        return;
    }
    if ([cooler isOn]) {
        [coolerSwitch setOn: YES];
    } else {
        [coolerSwitch setOn: NO];
    }
    double  temperature = [cooler getSetTemperature] - 273.15;
    temperatureLabel.text = [NSString stringWithFormat: @"%.1f°", temperature];
    temperatureSlider.value = temperature;
    actualtemperatureLabel.text = [NSString stringWithFormat: @"%.1f°", [cooler getActualTemperature] - 273.15];
}

- (void)temperatureChanged: (id)sender {
    if (sender == temperatureSlider) {
        double temperature = temperatureSlider.value;
        [cooler setTemperature: temperature + 273.15];
    }
    if (sender == coolerSwitch) {
        [cooler setOn: coolerSwitch.isOn];
    }
    [self updateCooler];
}

- (void)selectCcd: (int)ccdid {
    ccd = [camera getCcd: ccdid];
    ccdinfo = [ccd getInfo];
    exposure.frame.size = ccdinfo.size;
    exposure.frame.origin.x = 0;
    exposure.frame.origin.y = 0;
    NSString    *displayname = [[ccdinfo.name stringByRemovingPercentEncoding] lastPathComponent];
    NSLog(@"ccd name: %@", displayname);
    [ccdButton setTitle: displayname forState: UIControlStateNormal];
    [ccdButton setTitle: displayname forState: UIControlStateSelected];
    [shutterSwitch setEnabled: ccdinfo.shutter];
    if (!ccdinfo.shutter) {
        [shutterSwitch setOn: YES];
    }
    BOOL    hasCooler = [ccd hasCooler];
    [coolerSwitch setEnabled: hasCooler];
    [actualtemperatureLabel setEnabled: hasCooler];
    [temperatureSlider setEnabled: hasCooler];
    [temperatureLabel setEnabled: hasCooler];
    temperatureLabel.text = @"";
    actualtemperatureLabel.text = @"";
    if (hasCooler) {
        cooler = [ccd getCooler];
        [self updateCooler];
    } else {
        cooler = nil;
    }
}

- (void)selectBinning: (snowstarBinningMode *)binning {
    exposure.mode = binning;
    NSString    *binningdisplay = [NSString stringWithFormat: @"%dx%d", binning.x, binning.y];
    [binningButton setTitle: binningdisplay forState: UIControlStateNormal];
    [binningButton setTitle: binningdisplay forState: UIControlStateSelected];
}

- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController {
    popover = nil;
}

- (void)timerTick: (NSTimer *)timer {
    NSLog(@"timer tick");
    return;
    // cooler update
    if (allowTemperatureUpdate) {
        [self updateCooler];
    }
}

- (void)cancelExposureDisplay {
    [progresstimer invalidate];
    progresstimer = nil;
    [exposuretimer invalidate];
    exposuretimer = nil;
    [captureButton  setTitle: @"Capture" forState: UIControlStateNormal];
    [captureButton  setTitle: @"Capture" forState: UIControlStateSelected];
    [progressView setHidden: YES];
    progressView.progress = 0;
}

- (void)cancelExposure: (id)sender {
    NSLog(@"cancel");
    // find the two timers and invalidate them
    [self cancelExposureDisplay];
}

- (void)startExposure: (id)sender {
    NSLog(@"exposure.mode = %dx%d, size = %dx%d", exposure.mode.x, exposure.mode.y, exposure.frame.size.width, exposure.frame.size.height);
    int state = [ccd exposureStatus];
    switch (state) {
        case snowstarEXPOSING:
        case snowstarCANCELLING:
            return;
        case snowstarEXPOSED:
            [ccd cancelExposure];
        case snowstarIDLE:
            break;
    }
    
    allowTemperatureUpdate = NO;
    exposure.shutter = (shutterSwitch.isOn) ? snowstarShOPEN : snowstarShCLOSED;
    NSLog(@"exposure.shutter = %d", exposure.shutter);
    [ccd startExposure: exposure];
    exposureStart = [NSDate date];
    
    // timer to fire when the exposure is complete
    exposuretimer = [NSTimer timerWithTimeInterval: 1 target: self selector: @selector(retrieveImage:) userInfo: nil repeats: YES];
    [[NSRunLoop currentRunLoop] addTimer: exposuretimer forMode: NSDefaultRunLoopMode];
    
    // timer to fire at short intervals to update the progress indicator
    progresstimer = [NSTimer timerWithTimeInterval: 0 target: self selector: @selector(updateProgress:) userInfo:nil repeats:YES];
    [[NSRunLoop currentRunLoop] addTimer: progresstimer forMode: NSDefaultRunLoopMode];

    // make the progress view visible
    [progressView setHidden: NO];
    progressView.progress = 0;
    
    [captureButton  setTitle: @"Cancel" forState: UIControlStateNormal];
    [captureButton  setTitle: @"Cancel" forState: UIControlStateSelected];
}

- (void)toggleExposure: (id)sender {
    if (exposureStart) {
        [self cancelExposure: sender];
    } else {
        [self startExposure: sender];
    }
}

- (void)updateProgress: (NSTimer *)_progresstimer {
    if (exposureStart) {
        NSTimeInterval  interval = [[NSDate date] timeIntervalSinceDate: exposureStart];
        double progress = interval / (exposure.exposuretime + 1);
        if (progress > 1) { progress = 1; }
        [progressView setProgress: progress animated: YES];
    }
}

- (void)retrieveImage: (NSTimer *)_exposuretimer {
    int state = [ccd exposureStatus];
    if (state == snowstarEXPOSING) {
        NSLog(@"exposuretimer ticked");
        return;
    }
    if (state == snowstarEXPOSED) {
        NSLog(@"time to retrieve the image");
        snowstarImagePrx    *image = [ccd getImage];
        snowstarMutableImageFile    *file = [image file];
        NSLog(@"file size: %d, bytes received: %d", [image filesize], [file length]);
        snowstarShortImagePrx   *shortimage = [snowstarShortImagePrx checkedCast: image];
        if (nil == shortimage) {
            NSLog(@"image is not a short image");
        } else {
            snowstarImageSize   *size = [image size];
            snowstarMutableShortSequence    *imagedata = [shortimage getShorts];
            int l = [imagedata length];
            NSLog(@"got %d bytes of image data", l);
            
            // convert the image data into a CGDataProvider
            NSMutableData  *bytedata = [NSMutableData dataWithCapacity: l];
            for (int x = 0; x < size.width; x++) {
                for (int y = 0; y < size.height; y++) {
                    int i = x + y * size.width;
                    int j = x + (size.height - y - 1) * size.width;
                    ((unsigned char *)bytedata.bytes)[i] = ((unsigned short *)imagedata.bytes)[j] >> 8;
                }
            }
            CGDataProviderRef   data = CGDataProviderCreateWithCFData((CFDataRef)bytedata);
            
            // convert the image data into CGImage
            CGImageRef  cgimage = CGImageCreate(size.width, size.height, 8, 8, size.width, CGColorSpaceCreateDeviceGray(),
                          kCGBitmapByteOrderDefault,
                          data,
                          nil,
                          NO, kCGRenderingIntentDefault);
            UIImage *image = [UIImage imageWithCGImage: cgimage];
            imageView.image = image;
            [imageView sizeToFit];
            
            int xinset = 0;
            if (size.width < scrollView.frame.size.width) {
                xinset = (scrollView.frame.size.width - size.width) / 2;
            }
            int yinset = 0;
            if (size.height < scrollView.frame.size.height) {
                yinset = (scrollView.frame.size.height - size.height) / 2;
            }
            scrollView.contentInset = UIEdgeInsetsMake(yinset, xinset, yinset, xinset);
            scrollView.contentSize = CGSizeMake(size.width, size.height);
            [scrollView setNeedsLayout];
            NSLog(@"scrollview size: %f x %f",scrollView.frame.size.width, scrollView.frame.size.height);
            allowTemperatureUpdate = YES;
        }
    }
    [progressView setHidden: YES];
    exposureStart = nil;
    [self cancelExposureDisplay];
    [captureButton  setTitle: @"Capture" forState: UIControlStateNormal];
    [captureButton  setTitle: @"Capture" forState: UIControlStateSelected];
    [progresstimer invalidate];
    progresstimer = nil;
    exposuretimer = nil;
}

- (void)exposuretimeChanged:(id)sender {
    double  exposuretime = exposure.exposuretime;
    if (nil != sender) {
        NSLog(@"exposure time change");
        // read the exposure time from the exposure field
        NSNumberFormatter *formatter = [[NSNumberFormatter alloc] init];
        [formatter setNumberStyle:NSNumberFormatterDecimalStyle];
        NSNumber *exposurenumber = [formatter numberFromString: exposureField.text];
        exposuretime = [exposurenumber doubleValue];
        NSLog(@"exposure time: %f", exposuretime);
    }
    
    // check that the exposure time is valid
    if ((exposuretime <= 0) || (exposuretime > 1000)) {
        [captureButton setEnabled: NO];
        return;
    }
    exposure.exposuretime = exposuretime;
    
    if ((nil == camera) || (nil == ccd)) {
        [captureButton setEnabled: NO];
    }
    [captureButton setEnabled: YES];
}

@end
