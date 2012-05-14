<?php

function ocs_admin_message_form( $form, &$form_state, $nid = NULL, $op = NULL)
{
    $num_values = isset( $form_state['values']['num_values']) ? $form_state['values']['num_values'] : 0;

    $title = '';
    $format = '';
    if ( $op == 'edit') {
        $node = node_load( $nid);
        dpm( $node, 'node');
        if ( $node) {
            $title = $node->title;
            $format = $node->field_format['und'][0]['value'];
            if ( ! isset( $form_state['values']['num_values'])) {
                $num_values = count( $node->field_values['und']);
            }
        }
    }

    $form['#nid'] = $nid;
    $form['#op'] = $op;

    $form['title'] = array(
        '#type' => 'textfield',
        '#title' => 'Message Name',
        '#default_value' => $title,
    );

    $form['format'] = array(
        '#type' => 'textfield',
        '#title' => 'Format',
        '#default_value' => $format,
        '#description' => 'Enter message format in "printf" style. Use "%1, %2" to substitue numeric value.'.
            '<br>Ex) "You have remains %1 won."',
    );

    $counters = ocs_admin_get_counter_name_list();

    $form['valueset'] = array(
        '#type' => 'fieldset',
        '#title' => 'Variables',
        '#description' => 'Use following counter names to build the message string. : '. implode( ', ', array_keys( $counters)),
        '#prefix' => '<div id="message_format_div">',
        '#suffix' => '</div>',

    );

    $form['valueset']['num_values'] = array(
        '#type' => 'select',
        '#title' => 'Number of Variables',
        '#options' => array( 0, 1,2,3,4,5,6,7,8,9,10),
        '#default_value' => $num_values,
        '#ajax' => array(
            'callback' => 'ocs_admin_ajax_message_form_callback',
            'wrapper' => 'message_format_div',
        ),
    );

    for( $i =1; $i<= $num_values; $i++) {
        $form['valueset']['value'.$i] = array(
            '#type' => 'textfield',
            '#title' => 'Variable %'.$i,
            '#default_value' => isset( $node) ? $node->field_values['und'][$i-1]['value'] : '',
        );
    }

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

function ocs_admin_message_form_validate( $form, &$form_state)
{
    $values = $form_state['values'];

    if ( $values['op'] != 'Cancel') {
        foreach( array( 'title', 'format') as $key) {
            if ( empty( $values[$key])) {
                form_set_error( $key, $form[$key]['#title']. ' is empty'); 
            }
        }

        // check format string
        $format = str_word_count( $values['format'], 1, '%0123456789');
        foreach( $format as $fmt) {
            if ( substr( $fmt, 0, 1) == '%') {
                // check the variable number
                if ( substr( $fmt, 1) > $values['num_values']) {
                    form_set_error( 'format', t( 'Invalid variable "@v" in format string.', array( '@v' => $fmt)));
                }
            }
        }

        // check variables
        $counters = ocs_admin_get_counter_name_list();
        for( $i=1; $i <= $values['num_values']; $i++) {
            $words = str_word_count( $values['value'.$i], 1, '_');
            dpm( $words, 'value'.$i);
            foreach( $words as $word) {
                if ( ! array_key_exists( $word, $counters)) {
                    form_set_error( 'value'.$i, t( 'Invalid counter name "@name".', 
                        array( '@name' => $word)));
            }
            }
        }
    }
}


function ocs_admin_message_form_submit( $form, &$form_state)
{
    $values = $form_state['values'];

    if ( $values['op'] != 'Cancel') {   
        if ( $form['#op'] == 'edit') {
            $node = node_load( $form['#nid']);
        }
        else {
            $node = ocs_admin_get_new_node( 'ocs_messages');
        }
        $node->title = $values['title'];
        $node->field_format['und'][0]['value'] = $values['format'];

        for( $i=1; $i <= $values['num_values']; $i++) {
            $node->field_values['und'][$i-1]['value'] = $values['value'.$i];
        }

        node_save( $node);
    }

    $form_state['redirect'] = 'ocs_admin/config/ocs/message';
}


function ocs_admin_message_delete_confirm( $form, &$form_state, $nid)
{
	$name = isset( $_GET['name']) ? strip_tags($_GET['name']) : 'Message ID:' . $nid;

    $form['#nid'] = $nid;
    $form['#name'] = $name;

    $caption = t('<p><strong>Warning:</strong> Message "@name" will be deleted.</p>',
        array( '@name' => $name));

    $caption .= '<p>'. t('This action cannot be undone.') . '</p>';

    return confirm_form( $form, t('Are you sure to delete the message ?'),
        'ocs_admin/config/ocs/message/',
        $caption, t('Delete'));
}

function ocs_admin_message_delete_confirm_submit( $form, &$form_state)
{
    $nid = $form['#nid'];
    node_delete( $nid);

    drupal_set_message( t('The message "@name" has been delete.', array( '@name' => $form['#name'])));

    $form_state['redirect'] = 'ocs_admin/config/ocs/message';
}


