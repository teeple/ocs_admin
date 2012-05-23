<?php

define( 'OCS_DEFAULT_IP_ADDRESS', '0.0.0.0');
define( 'OCS_DEFAULT_USER_ACCOUNT', 'ocs');
define( 'OCS_DEFAULT_CONFIG_DIR', 'lua/script/uocs/ui');
define( 'OCS_DEFAULT_EVENT_FILE_NAME', 'OcsOpenItem.json');
define( 'OCS_DEFAULT_STATE_FILE_NAME', 'OcsStateConfig.json');
define( 'OCS_DEFAULT_SVC_PERM_FILE_NAME', 'OcsStatePermitConfig.json');
define( 'OCS_MESSAGE_FILE_NAME', 'OcsMessageConfig.json');

// for campaign rules
define( 'OCS_EVENT_ACTION_RULE_FILE_NAME', 'event_actions.rule');
define( 'OCS_ACTIVATION_BONUS_RULE_FILE_NAME', 'activation_bonus.rule');

// for VPN discount rules
define( 'OCS_PPS_ACTION_RULE_FILE_NAME', 'pps_actions.rule');
define( 'OCS_VPN_DISCOUNT_RULE_FILE_NAME', 'vpn_discount.rule');


define( 'OCS_ADMIN_NOTI_FLAG', 
'0|None
1|SMS
2|IVR
3|USSD');

define( 'OCS_ADMIN_PREF_LANG',
'0|Mongolia
1|English');

define( 'OCS_ADMIN_SUBS_TYPE',
'0|Postpaid
1|Prepaid
2|Hybrid');

define( 'OCS_ADMIN_CHARGING_TYPE',
'0|Deduct
1|Accumulate');

define( 'OCS_ADMIN_CARD_STATUS',
'0|Not Used
1|Used');

define( 'OCS_ADMIN_USABLE_TYPE',
'0|Unusable
1|Usable');

define( 'OCS_ADMIN_STATUS',
'0|Idle
1|Active');

function ocs_admin_config( $form, &$form_state) 
{
    $form['access'] = array(
        '#type' => 'fieldset',
        '#title' => 'OCS System Access',
        '#description' => 'Configure system access information',
        '#collapsible' => TRUE, 
        '#collapsed' => FALSE,
    );

    $form['access']['ip_address'] = array(
        '#type' => 'textfield',
        '#title' => 'IP Address',
        '#description' => 'Enter IP Address of OCS',
        '#default_value' => variable_get('ocs_admin_ip_address',
            OCS_DEFAULT_IP_ADDRESS),
    );

    $form['access']['user_account'] = array(
        '#type' => 'textfield',
        '#title' => 'Account Name',
        '#description' => 'Enter the name of OCS Account',
        '#default_value' => variable_get('ocs_admin_user_account',
            OCS_DEFAULT_USER_ACCOUNT),
    );

    $form['access']['user_password'] = array(
        '#type' => 'password',
        '#title' => 'Account Password',
        '#description' => 'Enter the passworf of OCS Account',
        '#default_value' => variable_get('ocs_admin_user_password',
            'Administrative Warning'),
    );

    $form['export'] = array(
        '#type' => 'fieldset',
        '#title' => 'Export Configuration',
        '#description' => 'Configure the directory and file names to export',
        '#collapsible' => TRUE, 
        '#collapsed' => FALSE,
    );

    $form['export']['config_directory'] = array(
        '#type' => 'textfield',
        '#title' => 'Configuration Directory',
        '#description' => 'Enter the directory path which store OCS configuration files',
        '#default_value' => variable_get('ocs_admin_config_directory',
            OCS_DEFAULT_CONFIG_DIR),
    );

    $form['export']['ocs_event_file'] = array(
        '#type' => 'textfield',
        '#title' => 'Event File Name',
        '#description' => t('Enter the file name of OCS Event Configuratin'),
        '#default_value' => variable_get('ocs_admin_ocs_event_file',
            OCS_DEFAULT_EVENT_FILE_NAME),
    );

    $form['export']['ocs_state_file'] = array(
        '#type' => 'textfield',
        '#title' => 'State Config File Name',
        '#description' => t('Enter the file name of OCS State Configuratin'),
        '#default_value' => variable_get('ocs_admin_ocs_state_file',
            OCS_DEFAULT_STATE_FILE_NAME),
    );

    $form['export']['ocs_svc_perm_file'] = array(
        '#type' => 'textfield',
        '#title' => 'Service Permission File Name',
        '#description' => t('Enter the file name of OCS Service Permission Configuratin'),
        '#default_value' => variable_get('ocs_admin_ocs_svc_perm_file',
            OCS_DEFAULT_SVC_PERM_FILE_NAME),
    );

    $form['export']['ocs_message_file'] = array(
        '#type' => 'textfield',
        '#title' => 'Notification Message Config File Name',
        '#description' => t('Enter the file name of OCS Notification Message Configuratin'),
        '#default_value' => variable_get('ocs_admin_ocs_message_file',
            OCS_MESSAGE_FILE_NAME),
    );

    $form['export']['ocs_event_action_rule_file'] = array(
        '#type' => 'textfield',
        '#title' => 'Event Action Rule File Name',
        '#description' => t('Enter the file name of Event Action Rule File'),
        '#default_value' => variable_get('ocs_admin_event_action_rule_file',
            OCS_EVENT_ACTION_RULE_FILE_NAME),
    );

    $form['export']['ocs_activation_bonus_file'] = array(
        '#type' => 'textfield',
        '#title' => 'Activation Bonus Rule File Name',
        '#description' => t('Enter the file name of Activation Bonus Rule File'),
        '#default_value' => variable_get('ocs_admin_activation_bonus_file',
            OCS_ACTIVATION_BONUS_RULE_FILE_NAME),
    );

    $form['export']['ocs_pps_action_rule_file'] = array(
        '#type' => 'textfield',
        '#title' => 'PPS Action Rule File Name',
        '#description' => t('Enter the file name of PPS Action Rule File'),
        '#default_value' => variable_get('ocs_admin_pps_action_rule_file',
            OCS_PPS_ACTION_RULE_FILE_NAME),
    );

    $form['export']['ocs_vpn_discount_rule_file'] = array(
        '#type' => 'textfield',
        '#title' => 'VPN Discount Rule File Name',
        '#description' => t('Enter the file name of VPN Discount Rule File'),
        '#default_value' => variable_get('ocs_admin_vpn_discount_rule_file',
            OCS_VPN_DISCOUNT_RULE_FILE_NAME),
    );

    $form['account'] = array(
        '#type' => 'fieldset',
        '#title' => 'Account Field Customization',
        '#description' => 'Customize available values of account fields',
        '#collapsible' => TRUE, 
        '#collapsed' => FALSE,
    );

    $form['account']['charging_type'] = array(
        '#type' => 'textarea',
        '#title' => 'Charging Type',
        '#description' => t('Input available values for "Charging Type" with format "value|name"'),
        '#default_value' => variable_get( 'ocs_admin_charging_type', OCS_ADMIN_CHARGING_TYPE),
    );

    $form['account']['subscription_type'] = array(
        '#type' => 'textarea',
        '#title' => 'Subscription Type',
        '#description' => t('Input available values for "Subscription Type" with format "value|name"'),
        '#default_value' => variable_get( 'ocs_admin_subscription_type', OCS_ADMIN_SUBS_TYPE),
    );

    $form['account']['preferred_language'] = array(
        '#type' => 'textarea',
        '#title' => 'Preferred Language',
        '#description' => t('Input available values for "Preferred Language" with format "value|name"'),
        '#default_value' => variable_get( 'ocs_admin_preferred_language', OCS_ADMIN_PREF_LANG),
    );

    $form['account']['notification_flag_set'] = array(
        '#type' => 'textarea',
        '#title' => 'Notification Flag',
        '#description' => t('Input available values for "Notification Flag" with format "value|name"'),
        '#default_value' => variable_get( 'ocs_admin_notification_flag_set', OCS_ADMIN_NOTI_FLAG),
    );

    $form['account']['card_status'] = array(
        '#type' => 'textarea',
        '#title' => 'Card Status',
        '#description' => t('Input available values for "Card Status" with format "value|name"'),
        '#default_value' => variable_get( 'ocs_admin_card_status', OCS_ADMIN_CARD_STATUS),
    );

    $form['account']['status'] = array(
        '#type' => 'textarea',
        '#title' => 'Account Status',
        '#description' => t('Input available values for "Account Status" with format "value|name"'),
        '#default_value' => variable_get( 'ocs_admin_status', OCS_ADMIN_STATUS),
    );

   $form['account']['usable_type'] = array(
        '#type' => 'textarea',
        '#title' => 'Usable Type',
        '#description' => t('Input available values for "Card Type Status" with format "value|name"'),
        '#default_value' => variable_get( 'ocs_admin_usable_type', OCS_ADMIN_USABLE_TYPE),
    );

   $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Save Configuration',
    );

    return $form;
}

/*
    submit OCS configuration
*/
function ocs_admin_config_submit( $form, &$form_state) 
{
    variable_set('ocs_admin_ip_address', $form_state['values']['ip_address']);
    variable_set('ocs_admin_user_account', $form_state['values']['user_account']);
    variable_set('ocs_admin_user_password', $form_state['values']['user_password']);
    variable_set('ocs_admin_config_directory', $form_state['values']['config_directory']);
    variable_set('ocs_admin_ocs_event_file', $form_state['values']['ocs_event_file']);
    variable_set('ocs_admin_ocs_state_file', $form_state['values']['ocs_state_file']);
    variable_set('ocs_admin_ocs_svc_perm_file', $form_state['values']['ocs_svc_perm_file']);
    variable_set('ocs_admin_ocs_message_file', $form_state['values']['ocs_message_file']);


    // campaign rules
    variable_set('ocs_admin_event_action_rule_file', $form_state['values']['ocs_event_action_rule_file']);
    variable_set('ocs_admin_activation_bonus_file', $form_state['values']['ocs_activation_bonus_file']);

    // VPN discount rules
    variable_set('ocs_admin_pps_action_rule_file', $form_state['values']['ocs_pps_action_rule_file']);
    variable_set('ocs_admin_vpn_discount_rule_file', $form_state['values']['ocs_vpn_discount_rule_file']);



    variable_set('ocs_admin_charging_type', $form_state['values']['charging_type']);
    variable_set('ocs_admin_subscription_type', $form_state['values']['subscription_type']);
    variable_set('ocs_admin_preferred_language', $form_state['values']['preferred_language']);
    variable_set('ocs_admin_notification_flag_set', $form_state['values']['notification_flag_set']);
    variable_set('ocs_admin_card_status', $form_state['values']['card_status']);
    variable_set('ocs_admin_status', $form_state['values']['status']);
    variable_set('ocs_admin_usable_type', $form_state['values']['usable_type']);

    drupal_set_message( t('The settings have been saved.'));
}


