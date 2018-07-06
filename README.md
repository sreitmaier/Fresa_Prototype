# fresa

Prototye scripts for a IoT h-da group project

## Setup

Install Dependencies with `npm install`.

Run Project in Root Folder with `npm start`.

## MQTT Guide

### Topic

The Topic Wording is very important to remember.

It has to be in this style: `location/ID`. For example `mini/1` or `speis/2`

The Topic is build in two parts:

1.  Location
2.  ID

The Location can be either `mini` or `speis`. These differentiate between our two delivery places Mini-Speisekammer und Speisekammer

The ID is a Number ascending from 0 to infinite.

In combination this creates the Topic Wording which has to be used throughout the project.

### Message

The Message of each MQTT Notification deliveres either the current status of each Mini or Speis or triggers an action at the station.

The Message can contain a variation of statuses:

**Status**

- `open`
- `reserved`
- `loaded`
- `empty loaded`

Each describes the status of the location it is reporting of.

**Actions**

- `open lock`

### Flags

To make sure to get the status of all locations, even when they are not reporting, the Messages have to have the Retain flag set to `true`.

## Delivery Page

In `qr` is the page located which the delivery driver is navigated to, to trigger an action which signals the station to play a sound, light up the LEDs and open the lock.

The QR-Codes content is a simple link to a webpage. This an example for local development:
`http://192.168.0.248:3001/qr?type=mini&id=0`

This URL also includes som Querys which is needed to identify the type and the ID of the station.
In future releases this can be requested with a random id which then receives the data of an database.
