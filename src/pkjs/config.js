module.exports = [
  {
    "type": "heading",
    "defaultValue": "App Configuration"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Display Settings"
      },
      {
        "type": "toggle",
        "messageKey": "SecondTick",
        "label": "Enable Seconds",
        "defaultValue": true
      },
      {
        "type": "slider",
        "messageKey": "StepType",
        "defaultValue": 10000,
        "label": "Slider",
        "description": "Step Goal (0 for daily average)",
        "min": 0,
        "max": 20000,
        "step": 1000
      },
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];