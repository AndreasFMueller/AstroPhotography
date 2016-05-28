//
//  SecondViewController.m
//  IceStar
//
//  Created by Andreas Müller on 31.01.14.
//  Copyright (c) 2014 Andreas Müller. All rights reserved.
//

#import "SecondViewController.h"
#import "ServiceTableViewController.h"
#import "AppDelegate.h"
#import "ServiceCell.h"

@interface SecondViewController ()

@end

@implementation SecondViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
	// Do any additional setup after loading the view, typically from a nib.
    
    // get the app delegate where you can find the service discover stuff
    NSLog(@"connect serviceController to tv %@", serviceTableView.description);
    
    [serviceTableView registerClass: ServiceCell.class forCellReuseIdentifier: @"ServiceListCell"];
    
    serviceController = [[UITableViewController alloc] initWithStyle: UITableViewStylePlain];
    serviceController.tableView = serviceTableView;
    
    AppDelegate *appdelegate = [[UIApplication sharedApplication] delegate];
    serviceTableView.dataSource = appdelegate.discover;
    NSLog(@"set data source of tableview to %@", serviceTableView.dataSource.description);
    [serviceTableView reloadData];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
