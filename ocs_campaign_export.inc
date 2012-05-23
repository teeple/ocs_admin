<?php

require_once( 'ocs_rule_campaign.inc');

function ocs_admin_campaign_export( $form, &$form_state)
{
    $form['desc'] = array(
        '#type' => 'markup',
        '#markup' => '<p>Check the Rule file contents that contains the active campaign rules.</p>',
    );

    $form['ocs'] = array(
        '#type' => 'fieldset',
        '#title' => 'OCS(Online Charging System) Access Info',
        '#description' => t('OCS Access Settings. It can be changed ').
            l( t('here'), '/admin/config/system/ocs_admin'),
        '#collapsible' => TRUE, 
        '#collapsed' => FALSE,
    );

    $ip_address = variable_get('ocs_admin_ip_address', 'Administrative Warning');
    $user_account = variable_get('ocs_admin_user_account', 'Administrative Warning');
    $user_password = variable_get('ocs_admin_user_password', 'Administrative Warning');
    $config_directory = variable_get('ocs_admin_config_directory', 'Administrative Warning');
    $action_file_name = variable_get('ocs_admin_event_action_rule_file', 'Administrative Warning');
    $bonus_file_name = variable_get('ocs_admin_activation_bonus_file', 'Administrative Warning');

    $form['ocs']['ip_address_markup'] = array(
        '#type' => 'item',
        '#title' => 'IP Address',
        '#markup' => $ip_address,
    );

    $form['ocs']['user_account'] = array(
        '#type' => 'item',
        '#title' => 'Account Name',
        '#markup' => $user_account,
    );

    $form['ocs']['config_directory'] = array(
        '#type' => 'item',
        '#title' => 'Configuration Directory',
        '#markup' => $config_directory,
    );

    $form['ocs']['event_action_file'] = array(
        '#type' => 'item',
        '#title' => 'Event Action Rule File Name',
        '#markup' => $action_file_name,
    );

    $form['ocs']['bonus_rule_file'] = array(
        '#type' => 'item',
        '#title' => 'Activation Bonus Rule File Name',
        '#markup' => $bonus_file_name,
    );

    list( $action, $bonus) = _campaign_export();

    $form['action_rule'] = array(
        '#type' => 'textarea',
        '#title' => 'event_actions.rule',
        '#value' => $action,
        '#rows' => 10,
    );

    $form['bonus_rule'] = array(
        '#type' => 'textarea',
        '#title' => 'activation_bonus.rule',
        '#value' => $bonus,
        '#rows' => 10,
    );

    $form['actions'] = array('#type' => 'actions');

    $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Upload Rule Files'),
    );

    $form['actions']['cancel'] = array(
        '#type' => 'submit',
        '#value' => t('Cancel'),
    );

    return $form;
}

function ocs_admin_campaign_export_submit( $form, &$form_state)
{
    $values = $form_state['values'];

    if ( $values['op'] != 'Cancel') {

        // write to file
        try{
            $ip_address = variable_get('ocs_admin_ip_address', 'Administrative Warning');
            $user = variable_get('ocs_admin_user_account', 'Administrative Warning');
            $passwd = variable_get('ocs_admin_user_password', 'Administrative Warning');
            $dir = variable_get('ocs_admin_config_directory', 'Administrative Warning');

            $files = array(
                array(
                    'fname' => variable_get('ocs_admin_ocs_event_action_rule_file', 'Administrative Warning'),
                    'value' => $values['action_rule'],
                    ),
                array(
                    'fname' => variable_get('ocs_admin_ocs_activation_bonus_file', 'Administrative Warning'),
                    'value' => $values['bonus_rule'],
                    ));

            foreach( $files as $file) {
                $fd = fopen( '/tmp/'. $file['fname'], 'w');
                fwrite( $fd, $file['value']);
                fclose( $fd);

                // ftp to host
                if ( ocs_admin_ftp_put( $ip_address, $user, $passwd, $dir, $file['fname'])) {
                    drupal_set_message( t('Rule file "@name" is updated in the OCS', array( '@name' => $file['fname'])));
                }
            }
        }
        catch( Exception $e) {
            drupal_set_message( $e->getMessage(), 'warning');       
        }
    }

    $form_state['redirect'] = 'ocs_admin/campaign/';
}

function _campaign_export()
{
    $campaigns = ocs_admin_campaigns_load_all();
    dpm( $campaigns, 'campaigns');
    
    $event_action = '[RULESET_EVENT_ACTIONS]
service_id , string(30)
event_name , string(32)
promotion_service_id , string(30) , result, mandatory
priority , int , result, mandatory
promotion_category , string(30), result
promotion_parameter, string(256) , result, mandatory

[EVENT_ACTIONS]';

    $newline = '
';

    $bonus_cond = '

[RULESET_ACTIVATION_BONUS_CONDITION]
!rules

[ACTIVATION_BONUS_CONDITION]
';

    $bonus_amount = '
[RULESET_ACTIVATION_BONUS_AMOUNT]
targetRule , string(32)
db_action , string(128)
!rules
 
[ACTIVATION_BONUS_AMOUNT]
';

    $bonus_factors = array();
    $amount_factors = array();

    foreach( $campaigns as $campaign) {
        $campaign_id = $campaign['campaign_id'];
        $campaign_rules = new OcsCampaignRules;
        $campaign_rules->load( $campaign_id);

        dpm( $campaign_rules->rules(), 'rules');

        // build event_actions.rule file
        $events = array();
        foreach( $campaign_rules->rules() as $rule) {
            $event_id = $rule->event_id();
            $event_node = node_load( $event_id);

            dpm( $event_node, 'event node');

            if ( ! in_array( $event_id, $events) ) {
                $events[] = $event_id;

                $property = json_decode($event_node->field_property['und'][0]['value'], TRUE);
                if ( !empty( $property)) {
                    $property_str = '';
                    foreach( $property as $key => $value) {
                        $property_str .= $key . '=' . $value . ' ';
                    }
                }

                $event_action .= t("
    @id: @property event_name=@event, promotion_service_id = giving_bonus , priority = 10 , promotion_parameter = 't_bonus_condition=activation_bonus_condition&t_bonus_amount=activation_bonus_amount'", 
                    array( '@id' => $campaign_id, '@event' => $event_node->title,
                        '@property' => isset( $property) ? $property_str.',' : ''));

            }

            // build activation_bonus.rule file
            $script = $rule->get_rule_script();
            dpm( $script, 'rule script');

            $bonus_cond .= $script['conditions']['rules'];
            $bonus_factors = array_merge( $bonus_factors, $script['conditions']['factors']);

            $bonus_amount .= $script['actions']['rules'];
            $amount_factors = array_merge( $amount_factors, $script['actions']['factors']);
        }
    }

    $rule_str = '';
    foreach( $bonus_factors as $key => $property) {
        $rule_str .= $key . ', ' . $property . $newline;
    }
    $bonus_cond = t( $bonus_cond, array('!rules' => $rule_str));

    $rule_str = '';
    foreach( $amount_factors as $key => $property) {
        $rule_str .= $key . ', ' . $property . $newline;
    }
    $bonus_amount = t( $bonus_amount, array('!rules' => $rule_str));

    return array( $event_action, $bonus_cond. $bonus_amount);
}

/*
function _get_rule_property( $param_id, $factor)
{
    // return 'int' for account counters
    if ( substr( $param_id, -7) == 'account') {
        return 'int ';
    }

    $items = entity_load( 'node', FALSE, array( 'type' => 'ocs_param', 'title' => $factor));
    $item = array_shift( $items);
    if ( ! empty($item)) {
        return $item->field_rule_property['und'][0]['value'];
    }
    return '';
}
*/


// get all active campaigns
function ocs_admin_campaigns_load_all()
{
    db_set_active('ocsdb');

    // get group information
    $query = db_select( 'ocs_campaign', 'c');
    $result = $query->condition( 'c.status', '1', '=')
        ->fields('c')
        ->execute()
        ->fetchAllAssoc( 'campaign_id', PDO::FETCH_ASSOC);

    db_set_active('default');
    return $result;

}
