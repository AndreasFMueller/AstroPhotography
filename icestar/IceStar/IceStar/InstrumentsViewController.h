//
//  InstrumentsViewController.h
//  IceStar
//
//  Created by Andreas Müller on 28.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "device.h"
#import "instruments.h"
#import "InstrumentsServiceController.h"

@interface InstrumentsViewController : UIViewController <UIPopoverControllerDelegate> {
    IBOutlet UIButton   *connectButton;
    IBOutlet UILabel    *connectionLabel;
    InstrumentsServiceController  *servicecontroller;
    snowstarModulesPrx  *modulesProxy;
    snowstarInstrumentsPrx  *instrumentsProxy;
}

- (IBAction)connectAction:(id)sender;
- (void)connectInstruments: (NSInteger)index;

@end
