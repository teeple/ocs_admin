<?php


module_load_include( 'inc', 'ocs_admin', 'ocs_campaign_action');

/*
    state transition rule edit form
*/
function ocs_admin_state_transition_form( $form, &$form_state, $nid= NULL, $op = NULL)
{
	$selected_action = isset( $form_state['values']['ocs_action_id']) ? $form_state['values']['ocs_action_id'] : NULL;

    $from_state = NULL;
    $to_state = NULL;

    if ( $op == 'edit') {
        $node = node_load( $nid);
        dpm( $node, 'transition');

        $from_state = $node->field_from_state['und'][0]['target_id'];
        $to_state = $node->field_to_state['und'][0]['target_id'];
        $selected_event = array();
        foreach( $node->field_event['und'] as $target) {
            $selected_event[] = $target['target_id'];
        }
//        dpm( $selected_event, 'selected');

        $action_id = $node->field_rule_id['und'][0]['value'];

        $edit_action = ocs_admin_campaign_action_load( $action_id);
        if ( ! isset( $selected_action)) $selected_action = $edit_action['ocs_action_id'];
        dpm( $edit_action, 'edit');
    }

    $form['#nid'] = $nid;
    $form['#op'] = $op;
    $form['#action_id'] = $action_id;

    // build account state list
    $items = entity_load( 'node', FALSE, array( 'type' => 'ocs_account_state'));
    $states = array();
    foreach( $items as $item) {
        $states[$item->nid] = $item->title;
    }

    // build OCS event list
    $items = entity_load( 'node', FALSE, array( 'type' => 'ocs_events'));
//    dpm( $items, 'events');
    $events = array();
    foreach( $items as $item) {
        $events[$item->nid] = $item->title;
    }


    $form['from_state'] = array(
        '#type' => 'select',
        '#title' => 'From State',
        '#options' => $states,
        '#default_value' => $from_state,
        '#disabled' => isset( $from_state),
    );

    $form['to_state'] = array(
        '#type' => 'select',
        '#title' => 'To State',
        '#options' => $states,
        '#default_value' => $to_state,
        '#disabled' => isset( $to_state),
    );

    $form['events'] = array(
        '#type' => 'checkboxes',
        '#title' => 'Triggering Events',
        '#options' => $events,
        '#default_value' => $selected_event,
    );

    // load action list
    $items = entity_load('node', FALSE, array('type' => 'ocs_actions'));
    $options = array();
    foreach( $items as $item) {
        $client = taxonomy_term_load( $item->field_ocs_client_id['und'][0]['tid']);
        $options[$client->name][$item->nid] = $item->field_description['und'][0]['safe_value'];
		if ( ! isset($selected_action)) {
            $selected_action = $item->nid;
            $form['#action_name'] = $item->title;
        }
        else if ( $selected_action == $item->nid) {
            $form['#action_name'] = $item->title;
        }
    }

//    dpm( $items, 'actions');
//    dpm( $selected_action, 'selected');

	$form['ocs_action_id'] = array(
		'#type' => 'select',
		'#title' => t('Action to add'),
		'#options' => $options,
		'#default_value' => $selected_action,
		'#description' => t('The Action shall be applied to the system.'),
		'#ajax' => array(
			'callback' => 'ocs_admin_ajax_action_param',
			'wrapper' => 'campaign_action_param_div',
			),
	);

	// get list of parameters
	$form['param'] = array(
		'#type' => 'fieldset',
		'#title' => 'Parameters',
		'#prefix' => '<div id="campaign_action_param_div">',
		'#suffix' => '</div>',
		);

	$action = node_load( $selected_action);
    $rule_id = 0;
    list( $paramlist, $options, $selected) = ocs_admin_get_param_list( $action, $rule_id);
//    dpm( $paramlist, 'paramlist');

    $list = array();
	foreach( $action->field_ocs_param_ref['und'] as $param) {
		$node = node_load( $param['target_id']);
		$key = $rule_id . ':' . $param['target_id'];
        $list[] = $key;

		$form['param'][$key] = ocs_admin_get_param_form( $paramlist, $key);
        if ( isset($edit_action) && isset( $edit_action['param'][$key]))  {
            $form['param'][$key]['#default_value'] = $edit_action['param'][$key]['value'];
        }
	}
	$form['#paramlist'] = $paramlist;
	$form['#param_name_list'] = $list;


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


function ocs_admin_state_transition_form_submit( $form, &$form_state)
{
    $values = $form_state['values'];

    if ( $values['op'] != 'Cancel') {
        if ( $form['#op'] == 'edit') {
            $node = node_load( $form['#nid']);
        }
        else {
            $node = ocs_admin_get_new_node( 'ocs_state_machine');
        }

        $node->title = $values['from_state'] . ' to ' . $values['to_state'];
        $node->field_from_state['und'][0]['target_id'] = $values['from_state'];
        $node->field_to_state['und'][0]['target_id'] = $values['to_state'];

        $node->field_event['und'] = array();
        foreach( $values['events'] as $target_id) 
        {
            if ( ! empty( $target_id)) {
                $node->field_event['und'][] = array( 'target_id' => $target_id);
            }
        }

        $node->field_ocs_action_ref['und'][0]['target_id'] = $values['ocs_action_id'];

        list( $param, $output) = ocs_admin_get_action_param_list( $form, $values);

        $action_id = $form['#action_id'];
        $action = array(
            'action_id' => $action_id,
            'rule_id' => 0,
            'ocs_action_id' => $values['ocs_action_id'],
            'action_name' => $output,
        );

        $action_id = ( ! empty( $action_id)) ?  ocs_admin_campaign_action_save( $action, $param)
            : ocs_admin_campaign_action_add( $action, $param);

        $node->field_rule_id['und'][0]['value'] = $action_id;
        $node->field_rule_param['und'][0]['value'] = $output;

        node_save( $node);
    }

    $form_state['redirect'] = 'ocs_admin/config/ocs/state/transition';
}

/*
** delete a transition rule
*/
function ocs_admin_transition_delete_confirm( $form, &$form_state, $nid)
{
    $node = node_load( $nid);
    $from_state = node_load( $node->field_from_state['und'][0]['target_id']);
    $to_state = node_load( $node->field_to_state['und'][0]['target_id']);

    $form['#nid'] = $nid;
    $form['#name'] = $from_state->title . ' to ' . $to_state->title;

    $caption = '<p>' . 
            t('<strong>Warning:</strong> Transition rule "@name" will be deleted.',
                array( '@name' => $form['#name'])) .
            '</p>';

    $caption .= '<p>'. t('This action cannot be undone.') . '</p>';

    return confirm_form( $form, t('Are you sure to delete the transition rule "@name"?',
        array( '@name' => $form['#name'])),
        'ocs_admin/config/ocs/state/transition',
        $caption, t('Delete'));
}

function ocs_admin_transition_delete_confirm_submit( $form, &$form_state)
{
    $nid = $form['#nid'];
    node_delete( $nid);

    drupal_set_message( t('The transition rule "@name" has been removed.',
			array( '@name' => $form['#name'])));

    $form_state['redirect'] = 'ocs_admin/config/ocs/state/transition';
}


