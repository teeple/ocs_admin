<?php

function ocs_admin_campaign_event_form( $form, &$form_state, $campaign_id)
{
    $form['#campaign_id'] = $campaign_id;

    $form['body'] = array(
        '#type' => 'markup',
        '#markup' => t( 'Select the event to add.'),
    );

    $items = entity_load('node', FALSE, array('type' => 'ocs_events'));
    $options = array();
    foreach( $items as $item) {
        $client = taxonomy_term_load( $item->field_ocs_client_id['und'][0]['tid']);
//        dpm( $client, 'client');
        $options[$client->name][$item->nid] = $item->title;
    }

	$form['event_id'] = array(
		'#type' => 'select',
		'#title' => t('React on event'),
		'#options' => $options,
		'#description' => t('Whenever the event occurs, rule evaluation is trigged'),
	);

    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save'),
    );

    $form['actions']['cancel'] = array(
        '#type' => 'submit',
        '#value' => t('Cancel'),
    );

    return $form;
}

function ocs_admin_campaign_event_form_submit( $form, &$form_state)
{
    $values = $form_state['values'];
    if ( $values['op'] != 'Cancel') {
        $event = array(
            'rule_id' => NULL,
            'rule_key' => t( 'campaign:@c:event:@e', 
                array( '@c' => $form['#campaign_id'], '@e' => $values['event_id'])),
        );

        ocs_admin_rule_save( $event);
    }

    $form_state['redirect'] = 'ocs_admin/campaign/' . $form['#campaign_id'] . '/rule';
}


/*
** delete an event
*/
function ocs_admin_campaign_event_delete_confirm( $form, &$form_state, $campaign_id, $event_id)
{
	$event_name = isset( $_GET['event_name']) ? $_GET['event_name'] : $event_id;

    $form['#campaign_id'] = $campaign_id;
    $form['#event_id'] = $event_id;
    $form['#event_name'] = $event_name;
    $caption = '<p>' . 
            t('<strong>Warning:</strong> The event "%key" and all of its rules will be deleted from the Campaign.',
                array( '%key' => $event_name)) .
            '</p>';

    $caption .= '<p>'. t('This action cannot be undone.') . '</p>';

    return confirm_form( $form, t('Are you sure to delete the event "%key"?',
        array( '%key' => $event_name)),
        'ocs_admin/campaign/' . $campaign_id . '/rule',
        $caption, t('Delete'));
}

function ocs_admin_campaign_event_delete_confirm_submit( $form, &$form_state)
{
    $campaign_id = $form['#campaign_id'];
    $event_id = $form['#event_id'];
    $event_name = $form['#event_name'];
    ocs_admin_campaign_delete_event( $campaign_id, $event_id);

    drupal_set_message( t('The event %key has been removed from the Campaign.', 
			array( '%key' => $event_name)));

    $form_state['redirect'] = 'ocs_admin/campaign/' . $campaign_id . '/rule';
}

function ocs_admin_campaign_delete_event( $campaign_id, $event_id)
{
    db_set_active( 'ocsdb');

	// delete event from campaign
	$result = db_delete( 'ocs_rules')
		->condition( 'rule_key', 'campaign:'.$campaign_id.':event:'.$event_id, '=')
		->execute();
	dpm( $result, 'ocs_campaign_event result');

    db_set_active( 'default');
}


