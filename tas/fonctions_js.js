function init()
{
	nbl=0;
	nbc=0;
	timer=0;
	logical_date=0;
	nb_grains_case=[0,0,0,0,0]; // nb_grains_case[i] contient le nombre de cases à i grains, i=4 pour >=4 grains
	
	table=[];
	elements_effondrables=[];
	
	couleurs=["white", "rgb(124,151,255)", "blue", "rgb(0,0,155)","black"];
	
	document.getElementById('menu1').style.display="block";
	document.getElementById('menu2').style.display="none";
	document.getElementById('menu3').style.display="none";
	document.getElementById('menu4').style.display="none";
	document.getElementById('menu5').style.display="none";
	document.getElementById('menu6').style.display="none";
	document.getElementById('menu7').style.display="none";
	document.getElementById('alerts').innerHTML="Veuillez entrer les dimensions de la grille.";
	document.getElementById('content').innerHTML="";
	
}

function getcouleur(i)
{
	return (i<4)?couleurs[i]:couleurs[4];
}

function setcouleur(i,j)
{
	var nbgrains=table[i][j];
	document.getElementById('td'+i+'_'+j).style.backgroundColor=getcouleur(nbgrains);
	if(nbgrains>0)
	{
		document.getElementById('td'+i+'_'+j).style.color=nbgrains>3?"white":"black";
	}
	
	else
	{
		document.getElementById('td'+i+'_'+j).style.backgroundColor="white";
	}
}

function menu1()
{
	if (parseInt(document.getElementById('nbc').value)>0 && parseInt(document.getElementById('nbl').value)>0)
	{
		document.getElementById('menu1').style.display="none";
		document.getElementById('menu2').style.display="inline-block";
		document.getElementById('menu3').style.display="inline-block";
		document.getElementById('menu4').style.display="inline-block";
		document.getElementById('menu5').style.display="inline-block";
		document.getElementById('alerts').innerHTML="<span style=\"color:green\">La grille est créée.</span>";

		nbl=parseInt(document.getElementById('nbl').value);
		nbc = parseInt(document.getElementById('nbc').value);
		
		var tableauhtml = ""; // c'est sale comme méthode...
		
		tableauhtml+="<table>";
		
		for(var i=0;i<nbl;i++)
		{
			tableauhtml+="<tr>";
			
			for(var j=0;j<nbc;j++)
			{
				tableauhtml+='<td value="(' + i + ', '+j+')" id="td'+i+'_'+j+'" onmouseover="infocase('+i+', '+j+')" onclick="actioncase('+i+', '+j+')">0</td>';	
			}
			
			tableauhtml+="</tr>";

		}
		
		tableauhtml+="</table>";
			
		document.getElementById('content').innerHTML=tableauhtml;
		style_cases();
		
		
		
	}
	
	else
	{
		document.getElementById('alerts').innerHTML='<span style="color:red;">Les dimensions ne vont pas...</span>';
	}
}

function actioncase(i,j)
{
	if(document.getElementById('menu2').style.display != "none")
	{
		changecase(i,j);	
	}	
	
	else
	{
		if(table[i][j]>=4)
		{
			var nb_effondrables = elements_effondrables.length;
			var case_choisie=0;
			
			for(case_choisie=0;case_choisie<nb_effondrables;case_choisie++)
			{
				var couple = elements_effondrables[case_choisie];
				if(i==couple[0] && j==couple[1])
				{
					break;	
				}
			}
		
			var intermediaire=elements_effondrables[case_choisie];
			elements_effondrables[case_choisie]=elements_effondrables[nb_effondrables -1];
			elements_effondrables[nb_effondrables -1]=intermediaire;	
		}
		ecroule(i,j);	
	}
}

function parserInt(a)
{
	if(a=="" || typeof(a) == "undefined")
	{
		return 0;	
	}	
	
	else
	{
		return parseInt(a);	
	}
}

function ecroule(i,j,option)
{
	
	var val=table[i][j];
	
	
	if(val>=4)
	{	

		if(val<8) elements_effondrables.pop();
		
		if(i-1 >= 0){
			table[i-1][j]++;
			if(table[i-1][j]==4) elements_effondrables.push([i-1,j]);
		}
		if(i+1 < nbl){
			table[i+1][j]++;
			if(table[i+1][j]==4) elements_effondrables.push([i+1,j]);
		}
		if(j-1 >= 0){
			table[i][j-1]++;
			if(table[i][j-1]==4) elements_effondrables.push([i,j-1]);
		}
		if(j+1 < nbc){
			table[i][j+1]++;
			if(table[i][j+1]==4) elements_effondrables.push([i,j+1]);
		}
		
		logical_date++;
		
		table[i][j]-=4;
		
		if (typeof(option) == 'undefined')
		{
			if(i-1 >= 0) {
				document.getElementById('td'+(i-1)+'_'+j).textContent=table[i-1][j];
				setcouleur(i-1,j);}
			if(i+1 < nbl) {
				document.getElementById('td'+(i+1)+'_'+j).textContent=table[i+1][j];
				setcouleur(i+1,j);}
			if(j-1 >= 0) {
				document.getElementById('td'+i+'_'+(j-1)).textContent=table[i][j-1];
				setcouleur(i,j-1);}
			if(j+1 < nbc){
				document.getElementById('td'+i+'_'+(j+1)).textContent=table[i][j+1];
				setcouleur(i,j+1);}	
		
			if(val=='4')
			{
				document.getElementById('td'+i+'_'+j).textContent='';
			}
		
			else
			{
				document.getElementById('td'+i+'_'+j).textContent=table[i][j];
			}
			
			document.getElementById('date').textContent=logical_date;
			setcouleur(i,j);
			enveloppeconvexe();
		
		}
		
				
	}
	
	else
	{
		var message='<span style="color:red">Cette case n\'est pas effondrable</span>';
		document.getElementById('alerts').innerHTML=message;	
	}
	
}

function menu2()
{
	var i=document.getElementById('vall').value;
	var j=document.getElementById('valc').value;
	var val=document.getElementById('valcase').value;
	
	document.getElementById('td'+i+'_'+j).textContent=parseInt(val);
}

function infocase(i,j)
{
	var valeur = document.getElementById('td'+i+'_'+j).textContent;
	var message='<span class="espace">ligne : ' + i + '</span><span class="espace">colonne : ' + j + '</span>Valeur : ' + parserInt(valeur);
	
	
	document.getElementById('alerts').innerHTML=message;	
}

function changecase(i,j)
{
	var valeur = parseInt(document.getElementById('td'+i+'_'+j).textContent);
	
	document.getElementById('vall').value=i;
	document.getElementById('valc').value=j;
	document.getElementById('valcase').value=valeur;
	
	document.getElementById('valcase').focus();

}

function clear_info()
{
	document.getElementById('alerts').innerHTML="";
}

function getsomme()
{
	var somme=0;	
	for(var i=0;i<nbl;i++)
		{
						
			for(var j=0;j<nbc;j++)
			{
				somme+=table[i][j];
			}
		}
		
		return somme;
}

function style_cases()
{
	var largeur = parseInt(document.getElementById('largeur').value)+ 'px';
	var longueur = parseInt(document.getElementById('longueur').value) + 'px'
	//couleur =document.getElementById('couleur').checked;
	
	for(var i=0;i<nbl;i++)
		{
						
			for(var j=0;j<nbc;j++)
			{
				document.getElementById('td'+i+'_'+j).style.width=largeur;
				document.getElementById('td'+i+'_'+j).style.minWidth=largeur;
				document.getElementById('td'+i+'_'+j).style.height=longueur;
//				if(couleur)
//				{
//					var maxi=getsomme();
//					var valeur = parseInt(document.getElementById('td'+i+'_'+j).textContent);
//					document.getElementById('td'+i+'_'+j).style.backgroundColor='rgb(255,'+ parseInt(valeur * 165/maxi) + ', 0)';
//				}
//				
//				else
//				{
//					document.getElementById('td'+i+'_'+j).style.background="none";
//				}
			}
			
			

		}
		
}

function alea()
{
	var	maxi=parseInt(document.getElementById('randmax').value);
	var mini = parseInt(document.getElementById('randmin').value);
	for(var i=0;i<nbl;i++)
		{
						
			for(var j=0;j<nbc;j++)
			{
				document.getElementById('td'+i+'_'+j).textContent=Math.floor(Math.random() * (maxi - mini + 1)) + mini;
					
			}
			
			

		}
		
}
	
function start()
{
	if(document.getElementById('menu2').style.display!="none")
	{
		document.getElementById('menu2').style.display="none";
		document.getElementById('menu3').style.display="none";
		document.getElementById('menu6').style.display="inline-block";
		document.getElementById('menu7').style.display="inline-block";
		document.getElementById('bouton_change').value="reset";
		
		var tableauhtml="<table>";
		
		table=new Array(nbl);

		for(var i=0;i<nbl;i++)
		{
			tableauhtml+="<tr>";
			table[i]=new Array(nbc);

			for(var j=0;j<nbc;j++)
			{
				table[i][j]=parserInt(document.getElementById('td'+i+'_'+j).textContent);
				
				var stylecss=' style="background-color:' + getcouleur(table[i][j]) + ';';
				if (table[i][j]>3)
				{
					stylecss+='color:white;';
					elements_effondrables.push([i,j]);
					
				}
					
				stylecss+='" ';
				
				
				tableauhtml+='<td value="(' + i + ', '+j+')" id="td'+i+'_'+j+'" '+ stylecss +'  onmouseover="infocase('+i+', '+j+')" onclick="actioncase('+i+', '+j+')">';
				tableauhtml+=(document.getElementById('td'+i+'_'+j).textContent == "0")?"":document.getElementById('td'+i+'_'+j).textContent;
				tableauhtml+= '</td>';
				
			}

			tableauhtml+="</tr>";

		}

		tableauhtml+="</table>";
		
		document.getElementById('content').innerHTML=tableauhtml;
		document.getElementById('date').textContent=logical_date;
		
		style_cases();
		enveloppeconvexe();
		
		
		
	}
	
	else
	{
		document.getElementById('bouton_change').value="Commencer la simulation";
		stop_timer();
		init();	
	}
}

function enveloppeconvexe(){
	var i_0 = nbl;
	var i_1 = 0;
	var j_0 = nbc;
	var j_1 = 0;
	var valeur = 0;
	var nb0 = 0;
	var nb1 = 0;
	var nb2 = 0;
	var nb3 = 0;
	var nb4plus = 0;
	
	for(var i=0;i<nbl;i++){
		for(var j=0;j<nbc;j++){
			
				valeur=table[i][j];
			
			if(valeur != 0){
				if(valeur == 1){
					nb1++;
				}
				if(valeur == 2){
					nb2++;
				}
				if(valeur == 3){
					nb3++;
				}
				if(valeur>=4){
					nb4plus++;	
				}

				if(i<i_0){
					i_0=i;
				}
				if(i>i_1){
					i_1=i;
				}
				if(j<j_0){
					j_0=j;
				}
				if(j>j_1){
					j_1=j;
				}
			}
			else
			{
				nb0++;
			}
		}
	}
	var x = i_1-i_0+1;
	var y = j_1-j_0+1;
	document.getElementById('largeurenv').textContent=x.toString();
	document.getElementById('longueurenv').textContent=y.toString();
	document.getElementById('nb0').textContent=nb0.toString();
	document.getElementById('nb1').textContent=nb1.toString();
	document.getElementById('nb2').textContent=nb2.toString();
	document.getElementById('nb3').textContent=nb3.toString();
	document.getElementById('nb4').textContent=nb4plus.toString();
	
	nb_grains_case=[nb0,nb1,nb2,nb3,nb4plus];
	

}

function jouerauhasard(option) // option=normal, on ne joue plus au hasard en fait !
{
	var	lookforfour = elements_effondrables.length;
	var	compteur=0;
	var valeur=0;
	var intermediaire=[];
	
		
	if(lookforfour==0)
	{
		stop_timer();
		enveloppeconvexe();
		return 0;
	}
		
		if(option=="normal")
		{
			var case_choisie=lookforfour-1;
			intermediaire=elements_effondrables[case_choisie];
		}
		
		else
		{
			var case_choisie = Math.floor((Math.random() * lookforfour) + 0);
		
			intermediaire=elements_effondrables[case_choisie];
			elements_effondrables[case_choisie]=elements_effondrables[lookforfour-1];
			elements_effondrables[lookforfour-1]=intermediaire;
		}
			
			var i=intermediaire[0];
			var j=intermediaire[1];
		
		
		if(option=="normal")
		{
			ecroule(i,j,option);
		}
						
		else
		{
			ecroule(i,j);
		}
						
		return 1;
	
}

function finir()
{
	stop_timer();
	
	while(jouerauhasard("normal") != 0);
	
	for(var i=0;i<nbl;i++)
	{
		
		for(var j=0;j<nbc;j++)
		{
			var valeur= table[i][j];
			document.getElementById('td'+i+'_'+j).textContent = valeur>0?valeur:"";
			setcouleur(i,j);
		}
	}
	
	enveloppeconvexe();
	document.getElementById('date').textContent=logical_date;

}

function lancertimer()
{
	if(timer==0)
	{
		var timer_intervalle = parserInt(document.getElementById('timer_intervalle').value);
		timer=setInterval(function () {jouerauhasard();}, timer_intervalle);	
		document.getElementById('letimer').value="Arrêter le timer";
	}	
	else
	{
		stop_timer();
	}	
	
}

function stop_timer()
{
	if(timer!=0)
	{
		window.clearInterval(timer);
		timer=0;
		document.getElementById('letimer').value="Commencer le timer";	
	}
}