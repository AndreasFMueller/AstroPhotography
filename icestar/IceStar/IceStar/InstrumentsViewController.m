//
//  InstrumentsViewController.m
//  IceStar
//
//  Created by Andreas Müller on 28.05.16.
//  Copyright © 2016 Andreas Müller. All rights reserved.
//

#import "InstrumentsViewController.h"
#import "InstrumentsServiceController.h"
#import "AppDelegate.h"
#import "ServiceCell.h"

@interface InstrumentsViewController ()

@end

@implementation InstrumentsViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/


- (IBAction)connectAction:(id)sender {
    servicecontroller = [[InstrumentsServiceController alloc] initWithStyle: UITableViewStylePlain];
    UIPopoverController *popController = [[UIPopoverController alloc] initWithContentViewController: servicecontroller];
    popController.popoverContentSize = CGSizeMake(390,400);
    [popController presentPopoverFromRect: connectButton.frame inView:self.view permittedArrowDirections: UIPopoverArrowDirectionAny animated: YES];
    popController.delegate = self;
    [servicecontroller.tableView registerClass: ServiceCell.class forCellReuseIdentifier: @"ServiceListCell"];
    AppDelegate *appdelegate = [[UIApplication sharedApplication] delegate];
    servicecontroller.tableView.dataSource = appdelegate.discover;
    servicecontroller.tableView.delegate = servicecontroller;
    servicecontroller.instrumentsViewController = self;
}

- (void)connectInstruments: (NSInteger)index {
    NSLog(@"connecting to %ld", (long)index);
    ServerDataSource    *datasource = servicecontroller.tableView.dataSource;
    ServerInfo  *serverinfo = [datasource serverAtIndex: index];
    snowstarInstrumentsPrx  *prx = [Connection instrumentsProxy: serverinfo];
    if (prx) {
        instrumentsProxy = prx;
        // dismiss the dialog
        [servicecontroller dismissViewControllerAnimated: YES completion: nil];
        connectionLabel.text = [NSString stringWithFormat: @"connected to Instruments service of %@ at %@:%d", serverinfo.servicename, serverinfo.hostname, serverinfo.port];
    }
}

@end
