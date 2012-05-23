<?php

function ocs_admin_family_page($form, &$form_statei, $group_id = NULL)
{
    $form = array();

    $form['group']['family'] = array (
        '#prefix' => '<div class="container-inline form-item">',
        '#suffix' => '</div>',
    );

    $form['group']['family']['item1'] = array(
        '#type' => 'item',
        '#title' => '<hr>',
    );


    $form['group']['family']['account_key'] = array(
        '#type' => 'textfield',
        '#title' => 'Family Number : ',
        '#size' => 20,
    );


    $form['group']['family']['account_search'] = array(
        '#type' => 'submit',
        '#value' => 'Search Account',
    );

    $form['group']['family']['item3'] = array(
        '#type' => 'item',
        '#description' => '<br>Click the <em>Search Account</em> button 
                           to find a family number',
    );

    $form['group']['family']['description'] = array(
        '#type' => 'textfield',
        '#title' => t('<br><br>Description :'),
        '#size' => 42,
    );


    $form['actions'] = array('#type' => 'actions');
    $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save'),
    );

    $form['actions']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Cancel'),
    );

    return $form;  
}
